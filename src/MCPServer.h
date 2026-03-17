#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

class BladeProcessor;

// ============================================================
// MCPServer  — JSON-RPC 2.0 over TCP (port 9000)
// Exposes RebornGuitar as an MCP Tool for LLM agents:
//   tools: set_threshold, set_scale, set_midi_channel,
//          get_status, trigger_note, set_poly_mode
// Wire protocol: newline-delimited JSON  {"jsonrpc":"2.0",...}
// ============================================================
class MCPServer : public juce::Thread,
                  private juce::InterprocessConnection
{
public:
    explicit MCPServer(BladeProcessor& p, int port = 9000);
    ~MCPServer() override;

    void start();  // Launch TCP listener
    void stop();

    // Broadcast state change to all connected clients
    void broadcastEvent(const juce::String& eventJson);

private:
    // ---- Thread ----
    void run() override;

    // ---- Connection callbacks ----
    void connectionMade()  override {}
    void connectionLost()  override {}
    void messageReceived(const juce::MemoryBlock& data) override;

    // ---- JSON-RPC dispatch ----
    juce::String handleRequest(const juce::String& json);
    juce::String methodSetThreshold   (const juce::var& params);
    juce::String methodSetScale       (const juce::var& params);
    juce::String methodSetMidiChannel (const juce::var& params);
    juce::String methodSetPolyMode    (const juce::var& params);
    juce::String methodGetStatus      (const juce::var& params);
    juce::String methodTriggerNote    (const juce::var& params);
    juce::String methodListTools      ();

    juce::String okResponse  (const juce::var& id, const juce::var& result);
    juce::String errResponse (const juce::var& id, int code, const juce::String& msg);

    BladeProcessor& processor;
    int port;
    std::unique_ptr<juce::StreamingSocket> server;
    juce::Array<std::unique_ptr<juce::StreamingSocket>> clients;
    juce::CriticalSection clientLock;
};
