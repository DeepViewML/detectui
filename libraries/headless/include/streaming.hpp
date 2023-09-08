#pragma once

#include "frame.h"
#include "streamingserver.h"
#include "videostream.h"
#include <atomic>
#include <future>
#include <gbm.h>
#include <vector>

class Streaming
{
public:
    using VSLEncoderPtr =
        std::unique_ptr<VSLEncoder, decltype(&vsl_encoder_release)>;

    Streaming(size_t inputWidth, size_t inputHeight);
    void SendFrame(gbm_bo* gbm_buffer);

private:
    VSLRect                        mCropRegions[4];
    StreamingServer::Tile          mTiles[4] = {StreamingServer::Tile::TopLeft,
                                                StreamingServer::Tile::TopRight,
                                                StreamingServer::Tile::BottomLeft,
                                                StreamingServer::Tile::BottomRight};
    std::vector<std::future<void>> mFutures;
    std::vector<VSLEncoderPtr>     mEncoders;
    std::vector<VSLFramePtr>       mEncodedFrames;
    VSLFramePtr                    mCurrentInputFrame;
    StreamingServer                mServer;
    size_t                         mNumberOfTiles;
    size_t                         mSequenceNumber = 0;
};