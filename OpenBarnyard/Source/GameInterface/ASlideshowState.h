#pragma once
#include "AGameState.h"
#include "GUI/AGUISlideshow.h"
#include "Input/AInputMapManager.h"

TOBJECT(ASlideshowState, AGameState, TFALSE)
{
public:
	ASlideshowState(const AGUISlideshow::Params& a_rSlideShowParams, AGameState* a_pNextGameState, TBOOL a_bShouldLocalise);

	virtual TBOOL ProcessInput(const Toshi::TInputInterface::InputEvent* a_pInputEvent) override;
	virtual TBOOL ProcessCommand(AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents) override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	virtual void OnInsertion() override;
	virtual void OnActivate() override;
	virtual void OnDeactivate() override;

	void AddSlide(const Toshi::TPString8 & a_rName);

private:
	AGUISlideshow m_Slideshow;
	AGameState* m_pNextState;
};
