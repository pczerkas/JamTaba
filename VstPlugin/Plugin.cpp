#include "Plugin.h"
#include <cmath>
#include <QWinWidget>
#include <QLabel>
#include <QSlider>
#include <QHBoxLayout>
#include <QStandardPaths>
#include "MainControllerVST.h"
#include "../NinjamController.h"
#include "../log/logHandler.h"
#include "Editor.h"

Q_LOGGING_CATEGORY(pluginVst, "plugin.vst")

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
    return new JamtabaPlugin (audioMaster);
}

JamtabaPlugin::JamtabaPlugin (audioMasterCallback audioMaster) :
    AudioEffectX (audioMaster, 0, 0),
    listEvnts(0),
    controller(nullptr),
    running(false),
    inputBuffer( DEFAULT_INPUTS*2),
    outputBuffer(DEFAULT_OUTPUTS*2),
    timeInfo(nullptr),
    hostWasPlayingInLastAudioCallBack(false)
{
    qCDebug(pluginVst) << "Plugin constructor...";
    setNumInputs (DEFAULT_INPUTS*2);
    setNumOutputs (DEFAULT_OUTPUTS*2);

    isSynth(false);
    canProcessReplacing(true);
    programsAreChunks(false);
    vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name

    canDoubleReplacing(false);

    setEditor(new VstEditor(this));

    //suspend();
    qCDebug(pluginVst) << "Plugin constructor done.";
}

bool JamtabaPlugin::hostIsPlaying() const{
    if(timeInfo){
        return (timeInfo->flags & kVstTransportPlaying) != 0;
    }
    return false;
}

int JamtabaPlugin::getHostBpm() const{
    if(timeInfo){
        return (int) timeInfo->tempo;
    }
    return 0;
}

VstInt32 JamtabaPlugin::fxIdle(){
    QApplication::processEvents();
    return 0;
}

bool JamtabaPlugin::needIdle(){
    return true;
}

JamtabaPlugin::~JamtabaPlugin ()
{
    qCDebug(pluginVst) << "Plugin destructor";
}

void JamtabaPlugin::initialize(){
    if(!isRunning()){
        if(!controller){
            qCDebug(pluginVst) << "Plugin initialize()...";
            QApplication::setApplicationName("Jamtaba 2");
            QApplication::setApplicationVersion(APP_VERSION);

            qInstallMessageHandler(customLogHandler);
            Persistence::Settings settings;//read from file in constructor
            settings.load();
            qCDebug(pluginVst)<< "Creating controller!";
            controller.reset( new MainControllerVST(settings, this));
            controller->configureStyleSheet("jamtaba.css");
            controller->setSampleRate(getSampleRate());
            controller->start();
            qCDebug(pluginVst)<< "Controller started!";
            running = true;
            qCDebug(pluginVst) << "Plugin initialize() done";
        }
    }
}

void JamtabaPlugin::open(){
    qCDebug(pluginVst) << "Plugin open()";

    //hostReportConnectionChanges = (bool)canHostDo((char*)"reportConnectionChanges");
    //hostAcceptIOChanges = (bool)canHostDo((char*)"acceptIOChanges");

    //reaper returns false :-/
//    char str[64];
//    getHostProductString(str);
//    if(!strcmp(str,"REAPER")) {
//        hostAcceptIOChanges = true;
//    }

//    opened=true;

}

void JamtabaPlugin::close()
{
    qCDebug(pluginVst) << "JamtabaPlugin::close()";
    if(editor){
        if(editor->isOpen()){
            editor->close();
        }
        ((VstEditor*)editor)->deleteMainWindow();
    }

    running = false;
    qCDebug(pluginVst) << "JamtabaPLugin::close() finished";
}

VstInt32 JamtabaPlugin::getNumMidiInputChannels()
{
    return 0;
}

VstInt32 JamtabaPlugin::getNumMidiOutputChannels()
{
    return 0;
}

bool JamtabaPlugin::getEffectName (char* name)
{
    if(!name)
        return false;
    vst_strncpy (name, "Jamtaba 2", kVstMaxEffectNameLen);
    return true;
}

bool JamtabaPlugin::getProductString (char* text)
{
    if(!text)
        return false;
    vst_strncpy (text, "Jamtaba 2", kVstMaxProductStrLen);
    return true;
}

bool JamtabaPlugin::getVendorString (char* text)
{
    if(!text)
        return false;
    vst_strncpy (text, "", kVstMaxVendorStrLen);
    return true;
}

VstInt32 JamtabaPlugin::getVendorVersion ()
{
    return 2;
}

VstInt32 JamtabaPlugin::canDo(char* text)
{
    if(!text){
        return 0;
    }
    if (!strcmp(text, "receiveVstTimeInfo")){
        return 1;
    }
    return 0;
}

void JamtabaPlugin::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
    if(!controller){
        return;
    }
    if(controller->isPlayingInNinjamRoom()){
        //++++++++++ sync ninjam BPM with host BPM ++++++++++++
        //ask timeInfo to VST host
        timeInfo = getTimeInfo ( kVstTransportPlaying | kVstTransportChanged | kVstTempoValid);
        if(transportStartDetectedInHost()){//user pressing play/start in host?
            NinjamControllerVST* ninjamController = dynamic_cast<NinjamControllerVST*>(controller->getNinjamController());
            if(ninjamController->isWaitingForHostSync()){
                ninjamController->syncWithHost();
            }
        }
    }

    //++++++++++ Audio processing +++++++++++++++
    inputBuffer.setFrameLenght(sampleFrames);
    for (int c = 0; c < inputBuffer.getChannels(); ++c) {
        memcpy(inputBuffer.getSamplesArray(c), inputs[c], sizeof(float) * sampleFrames);
    }

    outputBuffer.setFrameLenght(sampleFrames);
    outputBuffer.zero();

    controller->process(inputBuffer, outputBuffer, this->sampleRate);

    int channels = outputBuffer.getChannels();
    for (int c = 0; c < channels ; ++c) {
        memcpy(outputs[c], outputBuffer.getSamplesArray(c), sizeof(float) * sampleFrames);
    }

    //++++++++++++++++++++++++++++++
    hostWasPlayingInLastAudioCallBack = hostIsPlaying();
}


void JamtabaPlugin::setSampleRate(float sampleRate){
    qCDebug(pluginVst) << "JamtabaPlugin::setSampleRate()";
    this->sampleRate = sampleRate;
    if(controller){
        controller->setSampleRate(sampleRate);
    }

}


void JamtabaPlugin::suspend()
{
    qCDebug(pluginVst) << "JamtabaPLugin::suspend()";
    if(controller && controller->isPlayingInNinjamRoom()){
        controller->getNinjamController()->reset();//discard downloaded intervals
        controller->finishUploads();//send the last part of ninjam intervals
    }
}

void JamtabaPlugin::resume()
{
    qCDebug(pluginVst) << "JamtabaPLugin::resume()";
}
