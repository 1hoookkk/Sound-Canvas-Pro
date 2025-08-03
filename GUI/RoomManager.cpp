#include "RoomManager.h"
#include "Core/PluginProcessor.h"
#include "ArtefactLookAndFeel.h"

//==============================================================================
RoomManager::RoomManager(ARTEFACTAudioProcessor& processor)
    : audioProcessor(processor)
{
    createRoomTabs();
    setActiveRoom(SpectralRoom::ImageSynth);
}

RoomManager::~RoomManager() = default;

void RoomManager::paint(juce::Graphics& g)
{
    // Paint room tab background
    auto tabArea = getLocalBounds().removeFromTop(40);
    g.setColour(ArtefactLookAndFeel::kPanelBackground);
    g.fillRect(tabArea);
    
    // Paint separator line
    g.setColour(ArtefactLookAndFeel::kBevelDark);
    g.drawLine(0, 40, getWidth(), 40, 1.0f);
}

void RoomManager::resized()
{
    auto bounds = getLocalBounds();
    auto tabArea = bounds.removeFromTop(40);
    
    // Layout room tabs
    const int tabCount = static_cast<int>(roomTabs.size());
    const int tabWidth = tabArea.getWidth() / tabCount;
    
    for (int i = 0; i < tabCount; ++i)
    {
        if (roomTabs[i])
        {
            roomTabs[i]->setBounds(tabArea.removeFromLeft(tabWidth).reduced(2));
        }
    }
    
    // Layout room content area
    auto contentArea = bounds;
    
    // Show active room content
    if (imageSynthComponent && currentRoom == SpectralRoom::ImageSynth)
    {
        imageSynthComponent->setBounds(contentArea);
    }
    
    // TODO: Layout other rooms when implemented
    for (auto& room : rooms)
    {
        if (room)
        {
            room->setBounds(contentArea);
        }
    }
}

void RoomManager::buttonClicked(juce::Button* button)
{
    // Find which room tab was clicked
    for (int i = 0; i < static_cast<int>(roomTabs.size()); ++i)
    {
        if (roomTabs[i].get() == button)
        {
            setActiveRoom(static_cast<SpectralRoom>(i));
            break;
        }
    }
}

void RoomManager::setActiveRoom(SpectralRoom room)
{
    if (currentRoom == room) return;
    
    // Deactivate current room
    if (rooms[static_cast<int>(currentRoom)])
    {
        rooms[static_cast<int>(currentRoom)]->roomDeactivated();
    }
    
    currentRoom = room;
    
    // Activate new room
    if (rooms[static_cast<int>(currentRoom)])
    {
        rooms[static_cast<int>(currentRoom)]->roomActivated();
    }
    
    // Update tab appearance
    updateRoomVisibility();
}

void RoomManager::setImageSynthComponent(juce::Component* component)
{
    if (imageSynthComponent)
    {
        removeChildComponent(imageSynthComponent);
    }
    
    imageSynthComponent = component;
    
    if (imageSynthComponent)
    {
        addAndMakeVisible(imageSynthComponent);
        resized();
    }
}

void RoomManager::createRoomTabs()
{
    const std::array<juce::String, 3> roomNames = {
        "IMAGE SYNTH",    // Core room with everything integrated
        "SPECTRUM",       // Simplified spectral manipulation  
        "PROCESS"        // Essential spectral effects
    };
    
    for (int i = 0; i < static_cast<int>(roomTabs.size()); ++i)
    {
        roomTabs[i] = std::make_unique<juce::TextButton>(roomNames[i]);
        roomTabs[i]->addListener(this);
        addAndMakeVisible(roomTabs[i].get());
    }
}

void RoomManager::updateRoomVisibility()
{
    // Update tab button states
    for (int i = 0; i < static_cast<int>(roomTabs.size()); ++i)
    {
        if (roomTabs[i])
        {
            roomTabs[i]->setToggleState(i == static_cast<int>(currentRoom), 
                                       juce::dontSendNotification);
        }
    }
    
    // Show/hide room components
    if (imageSynthComponent)
    {
        imageSynthComponent->setVisible(currentRoom == SpectralRoom::ImageSynth);
    }
    
    for (int i = 0; i < static_cast<int>(rooms.size()); ++i)
    {
        if (rooms[i])
        {
            rooms[i]->setVisible(i == static_cast<int>(currentRoom));
        }
    }
    
    repaint();
}