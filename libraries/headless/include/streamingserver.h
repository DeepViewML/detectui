#pragma once

#include <cstring>
#include <glib-unix.h>
#include <glib.h>
#include <libsoup/soup.h>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include "frame.h"

class StreamingServer
{
public:
    using GMainLoopPtr =
        std::unique_ptr<GMainLoop, decltype(&g_main_loop_unref)>;
    using SoupServerPtr =
        std::unique_ptr<SoupServer, decltype(&g_object_unref)>;
    using GHashTablePtr =
        std::unique_ptr<GHashTable, decltype(&g_hash_table_destroy)>;
    using GTlsCertificatePtr =
        std::unique_ptr<GTlsCertificate, decltype(&g_object_unref)>;


    enum class Tile : size_t { TopLeft = 1, TopRight, BottomLeft, BottomRight };
    enum class Mode : size_t { SingleStream = 1, QuadTileStream = 4 };
    
    struct Frame {
        Tile   tile = Tile::TopLeft;
        VSLFramePtr frame;
        size_t sequenceNum;
    };


    StreamingServer(const std::string& pageTitle      = "Au-Zone demo",
                    size_t             httpsPort      = 443,
                    Mode               mode           = Mode::SingleStream,
                    size_t             tileWidth      = 1920,
                    size_t             tileHeight     = 1080,
                    size_t             fps            = 30,
                    size_t             maxConnections = 1,
                    bool               debugJs        = false);
    ~StreamingServer();

    void sendFrame(Frame frame);

    bool hasClients();
    size_t      getNumberOfTiles();

protected:
    static void websocketClosedCallback(SoupWebsocketConnection* connection,
                                        gpointer                 userData);
    static void websocketErrorCallback(SoupWebsocketConnection* connection,
                                        GError*                  error,
                                        gpointer                 userData);
    static void sendTile(gpointer key, gpointer value, gpointer userData);

    static void httpHandler(SoupServer*        soupServer,
                            SoupMessage*       message,
                            const char*        path,
                            GHashTable*        query,
                            SoupClientContext* clientContext,
                            gpointer           userData);

    static void websocketHandler(G_GNUC_UNUSED SoupServer* server,
                                 SoupWebsocketConnection*  connection,
                                 const char*               path,
                                 SoupClientContext*        clientContext,
                                 gpointer                  userData);

private:
    void        worker();
    std::thread workerThread;
    std::atomic<bool> mRun;
    std::mutex  mMutex;
    std::vector<Frame> mFramesToSend;

    GMainLoopPtr  mLoop;
    SoupServerPtr mSoupServer;
    GHashTablePtr mReceiverEntryTable;

    std::string mPageTitle;
    size_t      mHttpsPort;
    Mode        mMode;
    size_t      mFps;
    size_t      mTileWidth;
    size_t      mTileHeight;
    size_t      mMaxConnections;
    bool        mDebugJs;
};