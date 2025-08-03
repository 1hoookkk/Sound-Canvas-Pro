#pragma once

#include <JuceHeader.h>
#include <memory>

// Forward declarations
class ARTEFACTAudioProcessor;
class ArtefactLookAndFeel;

// Room enum for navigation (Simplified 3-room architecture)
enum class SpectralRoom
{
    ImageSynth = 0,    // Core image-to-sound synthesis + filters + effects
    Spectrum,          // Simplified spectral manipulation
    Process           // Essential spectral processing (freeze, smear, stretch)
};

// Base class for all rooms
class SpectralRoomComponent : public juce::Component
{
public:
    explicit SpectralRoomComponent(ARTEFACTAudioProcessor& processor) 
        : audioProcessor(processor) {}
    virtual ~SpectralRoomComponent() = default;
    
    // Called when room becomes active
    virtual void roomActivated() {}
    
    // Called when room becomes inactive
    virtual void roomDeactivated() {}
    
    // Get room name for tab display
    virtual juce::String getRoomName() const = 0;
    
    // Get room icon (optional)
    virtual juce::String getRoomIcon() const { return ""; }

protected:
    ARTEFACTAudioProcessor& audioProcessor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectralRoomComponent)
};

// Room manager handles switching between rooms
class RoomManager : public juce::Component,
                   public juce::Button::Listener
{
public:
    explicit RoomManager(ARTEFACTAudioProcessor& processor);
    ~RoomManager() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    
    // Room management
    void setActiveRoom(SpectralRoom room);
    SpectralRoom getActiveRoom() const { return currentRoom; }
    
    // Add existing components as rooms
    void setImageSynthComponent(juce::Component* component);
    
private:
    void createRoomTabs();
    void updateRoomVisibility();
    
    ARTEFACTAudioProcessor& audioProcessor;
    SpectralRoom currentRoom = SpectralRoom::ImageSynth;
    
    // Room tab buttons (3 rooms total)
    std::array<std::unique_ptr<juce::TextButton>, 3> roomTabs;
    
    // Room components (will be created as we implement each room)
    std::array<std::unique_ptr<SpectralRoomComponent>, 3> rooms;
    
    // Temporary: existing component integration
    juce::Component* imageSynthComponent = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoomManager)
};