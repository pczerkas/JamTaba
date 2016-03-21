#ifndef _LOCAL_INPUT_NODE_H_
#define _LOCAL_INPUT_NODE_H_

#include "AudioNode.h"

namespace Midi {
    class MidiMessage;
}

namespace Audio {

class LocalInputNode : public AudioNode
{
public:
    LocalInputNode(int parentChannelIndex, bool isMono = true);
    ~LocalInputNode();
    void processReplacing(const SamplesBuffer &in, SamplesBuffer &out, int sampleRate,
                                  const Midi::MidiMessageBuffer &midiBuffer) override;
    virtual int getSampleRate() const;

    int getChannels() const;

    bool isMono() const;

    bool isStereo() const;

    bool isNoInput() const;

    bool isMidi() const;

    bool isAudio() const;

    void setAudioInputSelection(int firstChannelIndex, int channelCount);

    void setMidiInputSelection(int midiDeviceIndex, int midiChannelIndex);

    void setToNoInput();

    int getMidiDeviceIndex() const;

    int getMidiChannelIndex() const;

    bool isReceivingAllMidiChannels() const;

    ChannelRange getAudioInputRange() const;

    int getGroupChannelIndex() const;

    const Audio::SamplesBuffer &getLastBuffer() const;

    void setProcessorsSampleRate(int newSampleRate);

    void closeProcessorsWindows();

    bool hasMidiActivity() const;

    quint8 getMidiActivityValue() const;

    void resetMidiActivity();

    void setMidiLowerNote(quint8 newLowerNote);

    void setMidiHigherNote(quint8 newHigherNote);

    quint8 getMidiLowerNote() const;

    quint8 getMidiHigherNote() const;

    void addProcessor(AudioNodeProcessor *newProcessor) override;

    void reset();

    /** local input tracks are always activated, so is possible play offline while listening to a room.
     The other tracks (ninjam tracks) are deactivated when the 'room preview' is started. */
    bool isActivated() const override;

private:
    //int globalFirstInputIndex; // store the first input index selected globally by users in preferences menu

    ChannelRange audioInputRange;
    // store user selected input range. For example, user can choose just the
    // right input channel (index 1), or use stereo input (indexes 0 and 1), or
    // use the channels 2 and 3 (the second input pair in a multichannel audio interface)

    int midiDeviceIndex; // setted when user choose MIDI as input method
    int midiChannelIndex;
    quint8 lastMidiActivity;// max velocity or control value
    quint8 midiLowerNote;
    quint8 midiHigherNote;

    int channelIndex; // the group index (a group contain N LocalInputAudioNode instances)

    enum InputMode {
        AUDIO, MIDI, DISABLED
    };

    InputMode inputMode = DISABLED;

    bool canAcceptMidiMessage(const Midi::MidiMessage &msg) const;

};

inline quint8 LocalInputNode::getMidiHigherNote() const
{
    return midiHigherNote;
}

inline quint8 LocalInputNode::getMidiLowerNote() const
{
    return midiLowerNote;
}

inline int LocalInputNode::getSampleRate() const
{
    return 0;
}

inline int LocalInputNode::getChannels() const
{
    return audioInputRange.getChannels();
}

inline int LocalInputNode::getMidiDeviceIndex() const
{
    return midiDeviceIndex;
}

inline int LocalInputNode::getMidiChannelIndex() const
{
    return midiChannelIndex;
}

inline ChannelRange LocalInputNode::getAudioInputRange() const
{
    return audioInputRange;
}

inline int LocalInputNode::getGroupChannelIndex() const
{
    return channelIndex;
}

inline const Audio::SamplesBuffer &LocalInputNode::getLastBuffer() const
{
    return internalOutputBuffer;
}

inline bool LocalInputNode::hasMidiActivity() const
{
    return lastMidiActivity > 0;
}

inline quint8 LocalInputNode::getMidiActivityValue() const
{
    return lastMidiActivity;
}

inline void LocalInputNode::resetMidiActivity()
{
    lastMidiActivity = 0;
}

inline bool LocalInputNode::isActivated() const
{
    return true;
}

}//namespace

#endif
