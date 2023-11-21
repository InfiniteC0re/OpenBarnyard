#include "pch.h"
#include <Toshi/Strings/TPString8.h>

TOSHI_NAMESPACE_USING

// Definitions of global pooled strings
TPString8 g_str_AnimControllerType;
TPString8 g_str_AnimObjTypes;
TPString8 g_str_AnimatableObjectType;
TPString8 g_str_AnimationTransition;
TPString8 g_str_AutoTransition;
TPString8 g_str_BlendInTime;
TPString8 g_str_BlendOutTime;
TPString8 g_str_Bone;
TPString8 g_str_BreakPoint;
TPString8 g_str_Breakpoint;
TPString8 g_str_CullWhenClipping;
TPString8 g_str_DefaultBlendInTime;
TPString8 g_str_DefaultBlendOutTime;
TPString8 g_str_DefaultFinishType;
TPString8 g_str_DefaultSpeed;
TPString8 g_str_DefaultWeight;
TPString8 g_str_EndOfAnim;
TPString8 g_str_ExportedName;
TPString8 g_str_FinishType;
TPString8 g_str_Include;
TPString8 g_str_Instant;
TPString8 g_str_LinkFrom;
TPString8 g_str_LinkTo;
TPString8 g_str_Loop;
TPString8 g_str_Model;
TPString8 g_str_Models;
TPString8 g_str_Name;
TPString8 g_str_NamedAnimation;
TPString8 g_str_NamedAnimationSet;
TPString8 g_str_Radius;
TPString8 g_str_Render;
TPString8 g_str_Reverse;
TPString8 g_str_Shadow;
TPString8 g_str_Speed;
TPString8 g_str_Static;
TPString8 g_str_Time;
TPString8 g_str_TransitionName;
TPString8 g_str_Type;
TPString8 g_str_UserRender;
TPString8 g_str_null;
TPString8 g_str_ParkManager;
TPString8 g_str_Start;
TPString8 g_str_ASlideshowState;
TPString8 g_str_bkg_by_legal1;
TPString8 g_str_bkg_Bink_Big;
TPString8 g_str_bkg_bluetongue;
TPString8 g_str_bkg_fmod;
TPString8 g_str_DialogInputMap;

static TPString8Initialiser::StringMap oInitStrMap[] = 
{
	{ &g_str_AnimControllerType,               "AnimControllerType" },
	{ &g_str_AnimObjTypes,                     "AnimObjTypes" },
	{ &g_str_AnimatableObjectType,             "AnimatableObjectType" },
	{ &g_str_AnimationTransition,              "AnimationTransition" },
	{ &g_str_AutoTransition,                   "AutoTransition" },
	{ &g_str_BlendInTime,                      "BlendInTime" },
	{ &g_str_BlendOutTime,                     "BlendOutTime" },
	{ &g_str_Bone,                             "Bone" },
	{ &g_str_BreakPoint,                       "BreakPoint" },
	{ &g_str_Breakpoint,                       "Breakpoint" },
	{ &g_str_CullWhenClipping,                 "CullWhenClipping" },
	{ &g_str_DefaultBlendInTime,               "DefaultBlendInTime" },
	{ &g_str_DefaultBlendOutTime,              "DefaultBlendOutTime" },
	{ &g_str_DefaultFinishType,                "DefaultFinishType" },
	{ &g_str_DefaultSpeed,                     "DefaultSpeed" },
	{ &g_str_DefaultWeight,                    "DefaultWeight" },
	{ &g_str_EndOfAnim,                        "EndOfAnim" },
	{ &g_str_ExportedName,                     "ExportedName" },
	{ &g_str_FinishType,                       "FinishType" },
	{ &g_str_Include,                          "Include" },
	{ &g_str_Instant,                          "Instant" },
	{ &g_str_LinkFrom,                         "LinkFrom" },
	{ &g_str_LinkTo,                           "LinkTo" },
	{ &g_str_Loop,                             "Loop" },
	{ &g_str_Model,                            "Model" },
	{ &g_str_Models,                           "Models" },
	{ &g_str_Name,                             "Name" },
	{ &g_str_NamedAnimation,                   "NamedAnimation" },
	{ &g_str_NamedAnimationSet,                "NamedAnimationSet" },
	{ &g_str_Radius,                           "Radius" },
	{ &g_str_Render,                           "Render" },
	{ &g_str_Reverse,                          "Reverse" },
	{ &g_str_Shadow,                           "Shadow" },
	{ &g_str_Speed,                            "Speed" },
	{ &g_str_Static,                           "Static" },
	{ &g_str_Time,                             "Time" },
	{ &g_str_TransitionName,                   "TransitionName" },
	{ &g_str_Type,                             "Type" },
	{ &g_str_UserRender,                       "UserRender" },
	{ &g_str_null,                             "null" },
	{ &g_str_ParkManager,                      "ParkManager" },
	{ &g_str_Start,                            "Start" },
	{ &g_str_ASlideshowState,                  "ASlideshowState" },
	{ &g_str_bkg_by_legal1,                    "bkg_by_legal1" },
	{ &g_str_bkg_Bink_Big,                     "bkg_Bink_Big" },
	{ &g_str_bkg_bluetongue,                   "bkg_bluetongue" },
	{ &g_str_bkg_fmod,                         "bkg_fmod" },
	{ &g_str_DialogInputMap,                   "DialogInputMap" },
};

static TPString8Initialiser oInitStrNode(oInitStrMap, 48, TTRUE);

