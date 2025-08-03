// Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "GUI/PluginEditor.h"

//==============================================================================
// Constructor and Destructor

ARTEFACTAudioProcessor::ARTEFACTAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
    #if ! JucePlugin_IsSynth
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
    #endif
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                     ),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Register as parameter listener for automatic parameter updates
    apvts.addParameterListener("masterGain", this);
    apvts.addParameterListener("paintActive", this);
    apvts.addParameterListener("processingMode", this);
}

ARTEFACTAudioProcessor::~ARTEFACTAudioProcessor()
{
    apvts.removeParameterListener("masterGain", this);
    apvts.removeParameterListener("paintActive", this);
    apvts.removeParameterListener("processingMode", this);
}

//==============================================================================
// Audio Processing Lifecycle

void ARTEFACTAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    // Prepare all processors
    forgeProcessor.prepareToPlay(sampleRate, samplesPerBlock);
    paintEngine.prepareToPlay(sampleRate, samplesPerBlock);
    sampleMaskingEngine.prepareToPlay(sampleRate, samplesPerBlock, 2); // Stereo
    audioRecorder.prepareToPlay(sampleRate, samplesPerBlock);
    
    // Set default active state based on current mode - START DISABLED to prevent feedback
    paintEngine.setActive(false);  // User must explicitly enable to prevent feedback loops
}

void ARTEFACTAudioProcessor::releaseResources()
{
    paintEngine.releaseResources();
    sampleMaskingEngine.releaseResources();
    audioRecorder.releaseResources();
    // Note: ForgeProcessor doesn't have releaseResources() method yet
}

//==============================================================================
// Parameter Management

juce::AudioProcessorValueTreeState::ParameterLayout ARTEFACTAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Master gain parameter
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "masterGain", "Master Gain", 0.0f, 2.0f, 0.7f));
    
    // Paint engine active parameter - START DISABLED to prevent feedback
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "paintActive", "Paint Active", false));
    
    // Processing mode parameter - default to Canvas mode (index 1)
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        "processingMode", "Processing Mode", 
        juce::StringArray{"Forge", "Canvas", "Hybrid"}, 1));
    
    return { parameters.begin(), parameters.end() };
}

void ARTEFACTAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "masterGain")
    {
        paintEngine.setMasterGain(newValue);
    }
    else if (parameterID == "paintActive")
    {
        paintEngine.setActive(newValue > 0.5f);
    }
    else if (parameterID == "processingMode")
    {
        int modeIndex = static_cast<int>(newValue);
        currentMode = static_cast<ProcessingMode>(modeIndex);
        
        // Update paint engine active state based on mode
        bool shouldBeActive = (currentMode == ProcessingMode::Canvas || 
                              currentMode == ProcessingMode::Hybrid);
        paintEngine.setActive(shouldBeActive);
    }
}

//==============================================================================
// Editor Management

juce::AudioProcessorEditor* ARTEFACTAudioProcessor::createEditor()
{
    return new ARTEFACTAudioProcessorEditor(*this);
}

//==============================================================================
// Bus Layout Support

bool ARTEFACTAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // Only mono/stereo supported
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

//==============================================================================
// State Management

void ARTEFACTAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save plugin state
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ARTEFACTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore plugin state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// Command Queue Management

bool ARTEFACTAudioProcessor::pushCommandToQueue(const Command& newCommand)
{
    return commandQueue.push(newCommand);
}

void ARTEFACTAudioProcessor::processCommands()
{
    // Process commands with a time limit to avoid blocking the audio thread
    // We allow up to 0.5ms for command processing (conservative limit)
    const double maxProcessingTimeMs = 0.5;
    
    commandQueue.processWithTimeLimit([this](const Command& cmd) {
        processCommand(cmd);
    }, maxProcessingTimeMs);
}

void ARTEFACTAudioProcessor::processCommand(const Command& cmd)
{
    // Route command based on type
    if (cmd.isForgeCommand())
    {
        processForgeCommand(cmd);
    }
    else if (cmd.isSampleMaskingCommand())
    {
        processSampleMaskingCommand(cmd);
    }
    else if (cmd.isPaintCommand())
    {
        processPaintCommand(cmd);
    }
    else if (cmd.isRecordingCommand())
    {
        processRecordingCommand(cmd);
    }
}

void ARTEFACTAudioProcessor::processForgeCommand(const Command& cmd)
{
    switch (cmd.getForgeCommandID())
    {
    case ForgeCommandID::StartPlayback:
        forgeProcessor.getVoice(cmd.intParam).start();
        break;
    case ForgeCommandID::StopPlayback:
        forgeProcessor.getVoice(cmd.intParam).stop();
        break;
    case ForgeCommandID::LoadSample:
        forgeProcessor.loadSampleIntoSlot(cmd.intParam, juce::File(cmd.stringParam));
        break;
    case ForgeCommandID::SetPitch:
        forgeProcessor.getVoice(cmd.intParam).setPitch(cmd.floatParam);
        break;
    case ForgeCommandID::SetSpeed:
        forgeProcessor.getVoice(cmd.intParam).setSpeed(cmd.floatParam);
        break;
    case ForgeCommandID::SetVolume:
        forgeProcessor.getVoice(cmd.intParam).setVolume(cmd.floatParam);
        break;
    case ForgeCommandID::SetDrive:
        forgeProcessor.getVoice(cmd.intParam).setDrive(cmd.floatParam);
        break;
    case ForgeCommandID::SetCrush:
        forgeProcessor.getVoice(cmd.intParam).setCrush(cmd.floatParam);
        break;
    case ForgeCommandID::SetSyncMode:
        forgeProcessor.getVoice(cmd.intParam).setSyncMode(cmd.boolParam);
        break;
    default:
        break;
    }
}

void ARTEFACTAudioProcessor::processSampleMaskingCommand(const Command& cmd)
{
    switch (cmd.getSampleMaskingCommandID())
    {
    case SampleMaskingCommandID::LoadSample:
        {
            juce::File sampleFile(cmd.stringParam);
            auto result = sampleMaskingEngine.loadSample(sampleFile);
            if (result.success)
            {
                DBG("SampleMaskingEngine: Loaded " << result.fileName << " (" << result.lengthSeconds << "s)");
                
                // NEW: Auto-detect tempo and enable sync for beatmakers
                auto tempoInfo = sampleMaskingEngine.detectSampleTempo();
                if (tempoInfo.confidence > 0.5f)
                {
                    DBG("SampleMaskingEngine: Detected tempo " << tempoInfo.detectedBPM << " BPM (confidence: " << tempoInfo.confidence << ")");
                    sampleMaskingEngine.enableTempoSync(true);
                }
                
                // NEW: Auto-start playback for immediate feedback (beatmaker friendly!)
                sampleMaskingEngine.startPlayback();
                DBG("SampleMaskingEngine: Auto-started playback");
            }
            else
            {
                DBG("SampleMaskingEngine: Load failed - " << result.errorMessage);
            }
        }
        break;
    case SampleMaskingCommandID::ClearSample:
        sampleMaskingEngine.clearSample();
        break;
    case SampleMaskingCommandID::StartPlayback:
        sampleMaskingEngine.startPlayback();
        break;
    case SampleMaskingCommandID::StopPlayback:
        sampleMaskingEngine.stopPlayback();
        break;
    case SampleMaskingCommandID::PausePlayback:
        sampleMaskingEngine.pausePlayback();
        break;
    case SampleMaskingCommandID::SetLooping:
        sampleMaskingEngine.setLooping(cmd.boolParam);
        break;
    case SampleMaskingCommandID::SetPlaybackSpeed:
        sampleMaskingEngine.setPlaybackSpeed(cmd.floatParam);
        break;
    case SampleMaskingCommandID::SetPlaybackPosition:
        sampleMaskingEngine.setPlaybackPosition(cmd.floatParam);
        break;
    case SampleMaskingCommandID::CreatePaintMask:
        {
            auto mode = static_cast<SampleMaskingEngine::MaskingMode>(static_cast<int>(cmd.floatParam));
            juce::uint32 maskId = sampleMaskingEngine.createPaintMask(mode, cmd.color);
            // Note: maskId could be stored for later reference if needed
        }
        break;
    case SampleMaskingCommandID::AddPointToMask:
        sampleMaskingEngine.addPointToMask(static_cast<juce::uint32>(cmd.intParam), cmd.x, cmd.y, cmd.pressure);
        break;
    case SampleMaskingCommandID::FinalizeMask:
        sampleMaskingEngine.finalizeMask(static_cast<juce::uint32>(cmd.intParam));
        break;
    case SampleMaskingCommandID::RemoveMask:
        sampleMaskingEngine.removeMask(static_cast<juce::uint32>(cmd.intParam));
        break;
    case SampleMaskingCommandID::ClearAllMasks:
        sampleMaskingEngine.clearAllMasks();
        break;
    case SampleMaskingCommandID::SetMaskMode:
        {
            auto mode = static_cast<SampleMaskingEngine::MaskingMode>(static_cast<int>(cmd.floatParam));
            sampleMaskingEngine.setMaskMode(static_cast<juce::uint32>(cmd.intParam), mode);
        }
        break;
    case SampleMaskingCommandID::SetMaskIntensity:
        sampleMaskingEngine.setMaskIntensity(static_cast<juce::uint32>(cmd.intParam), cmd.floatParam);
        break;
    case SampleMaskingCommandID::SetMaskParameters:
        // Use existing constructor pattern: SampleMaskingCommandID + int id + position data
        sampleMaskingEngine.setMaskParameters(static_cast<juce::uint32>(cmd.intParam), 
                                            cmd.x, cmd.y, cmd.pressure);
        break;
    case SampleMaskingCommandID::BeginPaintStroke:
        {
            auto mode = static_cast<SampleMaskingEngine::MaskingMode>(static_cast<int>(cmd.floatParam));
            sampleMaskingEngine.beginPaintStroke(cmd.x, cmd.y, mode);
        }
        break;
    case SampleMaskingCommandID::UpdatePaintStroke:
        sampleMaskingEngine.updatePaintStroke(cmd.x, cmd.y, cmd.pressure);
        break;
    case SampleMaskingCommandID::EndPaintStroke:
        sampleMaskingEngine.endPaintStroke();
        break;
    case SampleMaskingCommandID::SetCanvasSize:
        sampleMaskingEngine.setCanvasSize(cmd.floatParam, static_cast<float>(cmd.doubleParam));
        break;
    case SampleMaskingCommandID::SetTimeRange:
        sampleMaskingEngine.setTimeRange(cmd.floatParam, static_cast<float>(cmd.doubleParam));
        break;
    default:
        break;
    }
}

void ARTEFACTAudioProcessor::processPaintCommand(const Command& cmd)
{
    switch (cmd.getPaintCommandID())
    {
    case PaintCommandID::BeginStroke:
        paintEngine.beginStroke(PaintEngine::Point(cmd.x, cmd.y), cmd.pressure, cmd.color);
        break;
    case PaintCommandID::UpdateStroke:
        paintEngine.updateStroke(PaintEngine::Point(cmd.x, cmd.y), cmd.pressure);
        break;
    case PaintCommandID::EndStroke:
        paintEngine.endStroke();
        break;
    case PaintCommandID::ClearCanvas:
        paintEngine.clearCanvas();
        break;
    case PaintCommandID::SetPlayheadPosition:
        paintEngine.setPlayheadPosition(cmd.floatParam);
        break;
    case PaintCommandID::SetPaintActive:
        paintEngine.setActive(cmd.boolParam);
        break;
    case PaintCommandID::SetMasterGain:
        paintEngine.setMasterGain(cmd.floatParam);
        break;
    case PaintCommandID::SetFrequencyRange:
        paintEngine.setFrequencyRange(cmd.floatParam, static_cast<float>(cmd.doubleParam));
        break;
    case PaintCommandID::SetCanvasRegion:
        paintEngine.setCanvasRegion(cmd.x, cmd.y, cmd.floatParam, static_cast<float>(cmd.doubleParam));
        break;
    default:
        break;
    }
}

void ARTEFACTAudioProcessor::processRecordingCommand(const Command& cmd)
{
    switch (cmd.getRecordingCommandID())
    {
    case RecordingCommandID::StartRecording:
        audioRecorder.startRecording();
        DBG("AudioRecorder: Recording started via command");
        break;
    case RecordingCommandID::StopRecording:
        audioRecorder.stopRecording();
        DBG("AudioRecorder: Recording stopped via command");
        break;
    case RecordingCommandID::ExportToFile:
        if (cmd.stringParam[0] != '\0')
        {
            juce::File exportFile(cmd.getStringParam());
            auto format = static_cast<AudioRecorder::ExportFormat>(cmd.intParam);
            audioRecorder.exportToFile(exportFile, format);
            DBG("AudioRecorder: Export started to " << exportFile.getFullPathName());
        }
        break;
    case RecordingCommandID::SetRecordingFormat:
        // TODO: Implement format setting if needed
        break;
    case RecordingCommandID::SetRecordingDirectory:
        if (cmd.stringParam[0] != '\0')
        {
            juce::File directory(cmd.getStringParam());
            audioRecorder.setRecordingDirectory(directory);
            DBG("AudioRecorder: Recording directory set to " << directory.getFullPathName());
        }
        break;
    default:
        break;
    }
}

void ARTEFACTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    
    // CRITICAL: Skip all audio processing if paused (prevents feedback when minimized)
    if (audioProcessingPaused)
    {
        buffer.clear();  // Ensure silent output
        midi.clear();    // Clear any MIDI data
        return;
    }
    
    // Process all pending commands with time limit
    processCommands();

    // Update BPM if available from host
    if (auto playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
            {
                double hostBPM = *positionInfo->getBpm();
                if (std::abs(hostBPM - lastKnownBPM) > 0.1)
                {
                    lastKnownBPM = hostBPM;
                    forgeProcessor.setHostBPM(hostBPM);
                    
                    // NEW: Also update SampleMaskingEngine with host tempo
                    sampleMaskingEngine.setHostTempo(hostBPM);
                }
            }
            
            // NEW: Update SampleMaskingEngine with host position for tempo sync
            if (positionInfo->getPpqPosition().hasValue())
            {
                double ppqPos = *positionInfo->getPpqPosition();
                bool playing = positionInfo->getIsPlaying();
                sampleMaskingEngine.setHostPosition(ppqPos, playing);
            }
        }
    }

    // Process SampleMaskingEngine first (it can run alongside other modes)
    if (sampleMaskingEngine.hasSample())
    {
        juce::AudioBuffer<float> maskingBuffer(buffer.getNumChannels(), buffer.getNumSamples());
        maskingBuffer.clear();
        sampleMaskingEngine.processBlock(maskingBuffer);
        
        // Mix the masking engine output into the main buffer (increased level for beatmakers!)
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            buffer.addFrom(ch, 0, maskingBuffer, ch, 0, buffer.getNumSamples(), 0.8f); // Louder mix
        }
    }

    // Process audio based on current mode
    switch (currentMode)
    {
    case ProcessingMode::Canvas:
        // Canvas mode: Only PaintEngine
        paintEngine.processBlock(buffer);
        break;
        
    case ProcessingMode::Forge:
        // Forge mode: Only ForgeProcessor
        forgeProcessor.processBlock(buffer, midi);
        break;
        
    case ProcessingMode::Hybrid:
        // Hybrid mode: Mix both processors
        {
            juce::AudioBuffer<float> paintBuffer(buffer.getNumChannels(), buffer.getNumSamples());
            paintBuffer.clear();
            
            // Process paint engine into separate buffer
            paintEngine.processBlock(paintBuffer);
            
            // Process forge engine into main buffer
            forgeProcessor.processBlock(buffer, midi);
            
            // Mix the two signals (50/50 for now - could be parameterized)
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                buffer.addFrom(ch, 0, paintBuffer, ch, 0, buffer.getNumSamples(), 0.5f);
            }
        }
        break;
    }
    
    // Send processed audio to recorder for real-time capture
    audioRecorder.processBlock(buffer);
}

//==============================================================================
// Paint Brush System

void ARTEFACTAudioProcessor::setActivePaintBrush(int slotIndex)
{
    activePaintBrushSlot = juce::jlimit(0, 7, slotIndex);
}

void ARTEFACTAudioProcessor::triggerPaintBrush(float canvasY, float pressure)
{
    // Convert canvas Y position to frequency using PaintEngine's mapping
    float frequency = paintEngine.canvasYToFrequency(canvasY);
    
    // Convert frequency to semitones relative to 440Hz (A4)
    float semitones = 12.0f * std::log2(frequency / 440.0f);
    
    // Set pitch and trigger the active ForgeVoice
    auto& voice = forgeProcessor.getVoice(activePaintBrushSlot);
    if (voice.hasSample())
    {
        // Set pitch via command system for thread safety
        pushCommandToQueue(Command(ForgeCommandID::SetPitch, activePaintBrushSlot, semitones));
        
        // Set volume based on pressure
        float volume = juce::jlimit(0.0f, 1.0f, pressure);
        pushCommandToQueue(Command(ForgeCommandID::SetVolume, activePaintBrushSlot, volume));
        
        // Start playback
        pushCommandToQueue(Command(ForgeCommandID::StartPlayback, activePaintBrushSlot));
    }
}

void ARTEFACTAudioProcessor::stopPaintBrush()
{
    // Stop the active ForgeVoice
    pushCommandToQueue(Command(ForgeCommandID::StopPlayback, activePaintBrushSlot));
}

//==============================================================================
// Audio Processing Control (prevents feedback when minimized)

void ARTEFACTAudioProcessor::pauseAudioProcessing()
{
    audioProcessingPaused = true;
    
    // Stop all active voices immediately (prevent feedback loops)
    for (int i = 0; i < 8; ++i)
    {
        pushCommandToQueue(Command(ForgeCommandID::StopPlayback, i));
    }
    
    // Pause paint engine
    paintEngine.setActive(false);
    
    DBG("SpectralCanvas: Audio processing PAUSED - preventing feedback");
}

void ARTEFACTAudioProcessor::resumeAudioProcessing()
{
    audioProcessingPaused = false;
    
    // Restore paint engine state based on current mode and parameters
    bool shouldBeActive = (currentMode == ProcessingMode::Canvas || 
                          currentMode == ProcessingMode::Hybrid) &&
                         (apvts.getParameter("paintActive")->getValue() > 0.5f);
    paintEngine.setActive(shouldBeActive);
    
    DBG("SpectralCanvas: Audio processing RESUMED");
}

//==============================================================================
// Plugin Factory

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ARTEFACTAudioProcessor();
}
