//-----------------------------------------------------------------------------
// Every event the game sends to mods. Only included by AModEvents.h, which
// defines DECLARE_EVENT to number the events and to declare their structs.
//
//   DECLARE_EVENT( Class, Name, fields )
//
// declares ModEvent::Class::Name with these fields, the game fires it with
// FIRE_EVENT( Class::Name, values of the fields ).
//
// Events only report what has happened, a mod can't change what the game does
// through them. Use hooks (AModHooks.h) for that.
//
// NOTE: types declared in the global namespace need the :: prefix (::AGUI2*),
// otherwise they would mean the ModEvent namespace with the same name.
// NOTE: add new events at the end, the order is part of the mod API.
//-----------------------------------------------------------------------------

// The render interface has been created (ARenderer::CreateTRender)
DECLARE_EVENT( TRenderInterface, Created, Toshi::TRenderInterface* pRenderInterface; )

// The D3D device has been lost, the resources of the device have to be released
DECLARE_EVENT( TRenderInterface, DeviceLost )

// The D3D device has been reset, the resources of the device can be created again
DECLARE_EVENT( TRenderInterface, DeviceFound )

// ARenderer has created its resources (ARenderer::OnCreate)
DECLARE_EVENT( ARenderer, Created, ::ARenderer* pRenderer; )

// AGUI2 has been created, fonts and the root element can be used
DECLARE_EVENT( AGUI2, Created, ::AGUI2* pGUI2; )

// The AGUI2 root element has been rendered, called before the mouse cursor is drawn
DECLARE_EVENT( AGUI2, PostRender )

// The player has chosen to start a new game
DECLARE_EVENT( AGameLoader, NewGame )

// The story has advanced to a later act or tier after a quest
DECLARE_EVENT( AQuestManager, ProgressAdvanced, TINT iOldAct; TINT iOldTier; TINT iAct; TINT iTier; )

// The cow tipping game has ended (it's also the end of the story)
DECLARE_EVENT( ACowTippingMiniGameState, EndGame, ::ACowTippingMiniGameState* pState; )

// The loading screen has been shown or hidden
DECLARE_EVENT( ALoadScreen, StateChanged, TBOOL bLoading; )
