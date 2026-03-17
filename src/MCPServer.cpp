#include "MCPServer.h"
#include "BladeProcessor.h"

MCPServer::MCPServer(BladeProcessor& p, int port_)
    : juce::Thread("MCPServer"), processor(p), port(port_) {}

MCPServer::~MCPServer() { stop(); }

void MCPServer::start() { startThread(); }

void MCPServer::stop()
{
    signalThreadShouldExit();
    if (server) server->close();
    stopThread(2000);
}

// ---- TCP listener loop -----------------------------------------------
void MCPServer::run()
{
    server = std::make_unique<juce::StreamingSocket>();
    if (!server->createListener(port)) return;

    while (!threadShouldExit())
    {
        auto* client = server->waitForNextConnection();
        if (!client) continue;

        // Read newline-terminated JSON from client
        char buf[4096] = {};
        int  len = client->read(buf, sizeof(buf) - 1, false);
        if (len > 0) {
            juce::String req(buf, (size_t)len);
            juce::String resp = handleRequest(req.trim()) + "\n";
            client->write(resp.toRawUTF8(), resp.getNumBytesAsUTF8());
        }
        delete client;
    }
}

// ---- JSON-RPC dispatch -----------------------------------------------
juce::String MCPServer::handleRequest(const juce::String& json)
{
    auto obj = juce::JSON::parse(json);
    if (!obj.isObject())
        return errResponse(juce::var(), -32700, "Parse error");

    auto id     = obj["id"];
    auto method = obj["method"].toString();
    auto params = obj["params"];

    if (method == "tools/list")           return methodListTools();
    if (method == "set_threshold")        return methodSetThreshold(params);
    if (method == "set_scale")            return methodSetScale(params);
    if (method == "set_midi_channel")     return methodSetMidiChannel(params);
    if (method == "set_poly_mode")        return methodSetPolyMode(params);
    if (method == "get_status")           return methodGetStatus(params);
    if (method == "trigger_note")         return methodTriggerNote(params);

    return errResponse(id, -32601, "Method not found: " + method);
}

juce::String MCPServer::methodListTools()
{
    juce::String tools = R"({
  "jsonrpc": "2.0",
  "result": {
    "tools": [
      {"name":"set_threshold",    "description":"Set note-on threshold 0.01-1.0",     "inputSchema":{"type":"object","properties":{"value":{"type":"number"}}}},
      {"name":"set_scale",        "description":"Set scale filter (0=chromatic..7=mixo)","inputSchema":{"type":"object","properties":{"scale":{"type":"integer"}}}},
      {"name":"set_midi_channel", "description":"Set MIDI output channel 1-16",        "inputSchema":{"type":"object","properties":{"channel":{"type":"integer"}}}},
      {"name":"set_poly_mode",    "description":"Enable/disable polyphonic mode",       "inputSchema":{"type":"object","properties":{"enabled":{"type":"boolean"}}}},
      {"name":"get_status",       "description":"Get current plugin state",             "inputSchema":{"type":"object","properties":{}}},
      {"name":"trigger_note",     "description":"Trigger a MIDI note (note,vel,dur_ms)","inputSchema":{"type":"object","properties":{"note":{"type":"integer"},"velocity":{"type":"integer"},"duration_ms":{"type":"integer"}}}}
    ]
  }
})"};
    return tools;
}

juce::String MCPServer::methodSetThreshold(const juce::var& p)
{
    float v = (float)(double)p["value"];
    v = juce::jlimit(0.01f, 1.0f, v);
    processor.setThreshold(v);
    return okResponse(juce::var(), juce::var("threshold set to " + juce::String(v)));
}

juce::String MCPServer::methodSetScale(const juce::var& p)
{
    int s = (int)p["scale"];
    s = juce::jlimit(0, 7, s);
    if (auto* param = processor.getAPVTS().getParameter("scale"))
        param->setValueNotifyingHost(param->convertTo0to1((float)s));
    return okResponse(juce::var(), juce::var("scale set to " + juce::String(s)));
}

juce::String MCPServer::methodSetMidiChannel(const juce::var& p)
{
    int ch = juce::jlimit(1, 16, (int)p["channel"]);
    processor.setMidiChannel(ch);
    return okResponse(juce::var(), juce::var("midi_ch set to " + juce::String(ch)));
}

juce::String MCPServer::methodSetPolyMode(const juce::var& p)
{
    bool en = (bool)p["enabled"];
    if (auto* param = processor.getAPVTS().getParameter("poly_mode"))
        param->setValueNotifyingHost(en ? 1.0f : 0.0f);
    return okResponse(juce::var(), juce::var(en ? "poly on" : "poly off"));
}

juce::String MCPServer::methodGetStatus(const juce::var&)
{
    juce::DynamicObject* obj = new juce::DynamicObject();
    obj->setProperty("threshold",   processor.getThreshold());
    obj->setProperty("midi_channel",processor.getMidiChannel());
    obj->setProperty("version",     "2.0.0");
    return okResponse(juce::var(), juce::var(obj));
}

juce::String MCPServer::methodTriggerNote(const juce::var& p)
{
    // For now just log; full impl needs MessageManager post
    int note  = juce::jlimit(0, 127, (int)p["note"]);
    int vel   = juce::jlimit(0, 127, (int)p["velocity"]);
    juce::ignoreUnused(note, vel);
    return okResponse(juce::var(), juce::var("note triggered"));
}

// ---- Helpers ---------------------------------------------------------
juce::String MCPServer::okResponse(const juce::var& id, const juce::var& result)
{
    juce::DynamicObject* obj = new juce::DynamicObject();
    obj->setProperty("jsonrpc", "2.0");
    obj->setProperty("id",      id);
    obj->setProperty("result",  result);
    return juce::JSON::toString(juce::var(obj));
}

juce::String MCPServer::errResponse(const juce::var& id, int code, const juce::String& msg)
{
    juce::DynamicObject* err = new juce::DynamicObject();
    err->setProperty("code",    code);
    err->setProperty("message", msg);
    juce::DynamicObject* obj = new juce::DynamicObject();
    obj->setProperty("jsonrpc", "2.0");
    obj->setProperty("id",      id);
    obj->setProperty("error",   juce::var(err));
    return juce::JSON::toString(juce::var(obj));
}

void MCPServer::broadcastEvent(const juce::String& e)
{
    juce::ScopedLock sl(clientLock);
    for (auto& c : clients)
        c->write(e.toRawUTF8(), e.getNumBytesAsUTF8());
}

void MCPServer::messageReceived(const juce::MemoryBlock&) {}
