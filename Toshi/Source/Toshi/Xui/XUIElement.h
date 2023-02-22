#pragma once

// Class names
#define XUI_CLASS_ELEMENT           L"XuiElement"
#define XUI_CLASS_CANVAS            L"XuiCanvas"
#define XUI_CLASS_EFFECT            L"XuiEffect"
#define XUI_CLASS_TEXTURE_EFFECT    L"XuiTextureEffect"
#define XUI_CLASS_GRAYSCALE_EFFECT  L"XuiGrayscaleEffect"
#define XUI_CLASS_RECOLOR_EFFECT    L"XuiRecolorEffect"
#define XUI_CLASS_BRIGHTPASS_EFFECT L"XuiBrightPassEffect"
#define XUI_CLASS_EDGE_EFFECT       L"XuiEdgeEffect"
#define XUI_CLASS_HVBLUR_EFFECT     L"XuiHVBlurEffect"
#define XUI_CLASS_HBLUR_EFFECT      L"XuiHBlurEffect"
#define XUI_CLASS_VBLUR_EFFECT      L"XuiVBlurEffect"
#define XUI_CLASS_FIGURE            L"XuiFigure"
#define XUI_CLASS_CONTROL           L"XuiControl"
#define XUI_CLASS_SCENE             L"XuiScene"
#define XUI_CLASS_BUTTON            L"XuiButton"
#define XUI_CLASS_NAVBUTTON         L"XuiNavButton"
#define XUI_CLASS_BACKBUTTON        L"XuiBackButton"
#define XUI_CLASS_CHECKBOX          L"XuiCheckbox"
#define XUI_CLASS_RADIOBUTTON       L"XuiRadioButton"
#define XUI_CLASS_RADIOGROUP        L"XuiRadioGroup"
#define XUI_CLASS_TEXT              L"XuiText"
#define XUI_CLASS_TEXT_PRESENTER    L"XuiTextPresenter"
#define XUI_CLASS_VISUAL            L"XuiVisual"
#define XUI_CLASS_GROUP             L"XuiGroup"
#define XUI_CLASS_SOUND             L"XuiSound"
#define XUI_CLASS_SOUND_XACT        L"XuiSoundXACT"
#define XUI_CLASS_SOUND_XAUDIO      L"XuiSoundXAudio"
#define XUI_CLASS_VIDEO             L"XuiVideo"
#define XUI_CLASS_TRANSITION        L"XuiTransition"
#define XUI_CLASS_LIST              L"XuiList"
#define XUI_CLASS_COMMONLIST        L"XuiCommonList"
#define XUI_CLASS_LISTITEM          L"XuiListItem"
#define XUI_CLASS_SCROLLEND         L"XuiScrollEnd"
#define XUI_CLASS_SCROLLBAR         L"XuiScrollBar"
#define XUI_CLASS_PROGRESSBAR       L"XuiProgressBar"
#define XUI_CLASS_SLIDER            L"XuiSlider"
#define XUI_CLASS_CARET             L"XuiCaret"
#define XUI_CLASS_EDIT              L"XuiEdit"
#define XUI_CLASS_IMAGE             L"XuiImage"
#define XUI_CLASS_IMAGE_PRESENTER   L"XuiImagePresenter"
#define XUI_CLASS_MESSAGEBOX        L"XuiMessageBox"
#define XUI_CLASS_LABEL             L"XuiLabel"
#define XUI_CLASS_TABSCENE          L"XuiTabScene"
#define XUI_CLASS_MESSAGEBOXBUTTON  L"XuiMessageBoxButton"
#define XUI_CLASS_NINE_GRID         L"XuiNineGrid"

// XuiElement property names
#define XUI_ELEM_PROP_ID        L"Id"
#define XUI_ELEM_PROP_WIDTH     L"Width"
#define XUI_ELEM_PROP_HEIGHT    L"Height"
#define XUI_ELEM_PROP_POSITION  L"Position"
#define XUI_ELEM_PROP_SCALE     L"Scale"
#define XUI_ELEM_PROP_ROTATION  L"Rotation"

// XuiElement APIs
#define XUI_ANCHOR_NONE         0x0000
#define XUI_ANCHOR_LEFT         0x0001
#define XUI_ANCHOR_TOP          0x0002
#define XUI_ANCHOR_RIGHT        0x0004
#define XUI_ANCHOR_BOTTOM       0x0008
#define XUI_ANCHOR_HCENTER      0x0010
#define XUI_ANCHOR_VCENTER      0x0020
#define XUI_ANCHOR_XSCALE       0x0040
#define XUI_ANCHOR_YSCALE       0x0080

// XuiFigure APIs
enum XUI_FILL_TYPE
{
    XUI_FILL_NONE = 0,
    XUI_FILL_SOLID = 1,
    XUI_FILL_LINEAR_GRADIENT = 2,
    XUI_FILL_RADIAL_GRADIENT = 3,
    XUI_FILL_TEXTURE = 4
};

// XuiImageElement APIs
enum XUI_CONTENT_TYPE
{
    XUI_CONTENT_NONE,       // there is no content
    XUI_CONTENT_OBJ,        // content is a XUI object
    XUI_CONTENT_BRUSH,      // content is a XUI brush
};

// Control visual states: normal/focus
#define XUI_NORMAL                  L"Normal"                   // required for all control visuals
#define XUI_ENDNORMAL               L"EndNormal"

#define XUI_FOCUS                   L"Focus"                    // when not specified by the visual, try XUI_NORMAL
#define XUI_ENDFOCUS                L"EndFocus"
#define XUI_INITFOCUS               L"InitFocus"                // when not specified by the visual, try XUI_FOCUS
#define XUI_ENDINITFOCUS            L"EndInitFocus"
#define XUI_KILLFOCUS               L"KillFocus"                // when not specified by the visual, try XUI_NORMAL
#define XUI_ENDKILLFOCUS            L"EndKillFocus"

#define XUI_NORMALDISABLE           L"NormalDisable"            // normal, disabled: when not specified by the visual, try XUI_NORMAL
#define XUI_ENDNORMALDISABLE        L"EndNormalDisable"
#define XUI_FOCUSDISABLE            L"FocusDisable"             // focused, disabled: when not specified by the visual, try XUI_FOCUS
#define XUI_ENDFOCUSDISABLE         L"EndFocusDisable"
#define XUI_INITFOCUSDISABLE        L"InitFocusDisable"         // initial focus, disabled: when not specified by the visual, try XUI_FOCUSDISABLE
#define XUI_ENDINITFOCUSDISABLE     L"EndInitFocusDisable"

// Control visual states: press
#define XUI_PRESS                   L"Press"                    // focused, pressed: when not specified by the visual, try XUI_FOCUS
#define XUI_ENDPRESS                L"EndPress"
#define XUI_NORMALPRESS             L"NormalPress"              // normal, pressed: when not specified by the visual, try XUI_PRESS
#define XUI_ENDNORMALPRESS          L"EndNormalPress"
#define XUI_PRESSDISABLE            L"PressDisable"             // pressed, disabled: when not specified by the visual, try XUI_NORMALPRESS
#define XUI_ENDPRESSDISABLE         L"EndPressDisable"

// Control visual states: checked
#define XUI_NORMALCHECK             L"NormalCheck"              // normal, checked: when not specified by the visual, try XUI_NORMAL
#define XUI_ENDNORMALCHECK          L"EndNormalCheck"
#define XUI_FOCUSCHECK              L"FocusCheck"               // focused, checked: when not specified by the visual, try XUI_NORMALCHECK
#define XUI_ENDFOCUSCHECK           L"EndFocusCheck"

#define XUI_INITFOCUSCHECK          L"InitFocusCheck"           // initial focus, checked: when not specified by the visual, try XUI_FOCUSCHECK
#define XUI_ENDINITFOCUSCHECK       L"EndInitFocusCheck"

#define XUI_NORMALCHECKDISABLE      L"NormalCheckDisable"       // normal, checked, disabled: when not specified by the visual, try XUI_NORMALCHECK
#define XUI_ENDNORMALCHECKDISABLE   L"EndNormalCheckDisable"
#define XUI_FOCUSCHECKDISABLE       L"FocusCheckDisable"        // focused, checked, disabled: when not specified by the visual, try XUI_FOCUSCHECK
#define XUI_ENDFOCUSCHECKDISABLE    L"EndFocusCheckDisable"
#define XUI_INITFOCUSCHECKDISABLE   L"InitFocusCheckDisable"    // initial focus, checked, disabled: when not specified by the visual, try XUI_FOCUSCHECKDISABLE
#define XUI_ENDINITFOCUSCHECKDISABLE L"EndInitFocusCheckDisable"

// Control visual states: selected
// NOTE: selected applies only to normal; focused and selected is the same as focused
#define XUI_NORMALSEL               L"NormalSel"                // normal, selected: when not specified by the visual, try XUI_NORMAL
#define XUI_ENDNORMALSEL            L"EndNormalSel"
#define XUI_NORMALSELCHECK          L"NormalSelCheck"           // normal, checked, selected: when not specified by the visual, try XUI_NORMALSEL
#define XUI_ENDNORMALSELCHECK       L"EndNormalSelCheck"

#define XUI_NORMALSELDISABLE        L"NormalSelDisable"         // normal, selected, disabled: when not specified by the visual, try XUI_NORMALSEL
#define XUI_ENDNORMALSELDISABLE     L"EndNormalSelDisable"
#define XUI_NORMALSELCHECKDISABLE   L"NormalSelCheckDisable"    // normal, checked, selected, disabled: when not specified by the visual, try XUI_NORMALSELDISABLE
#define XUI_ENDNORMALSELCHECKDISABLE L"EndNormalSelCheckDisable"

// Control visual states: scroll ends
// NOTE: scrolling applies only when more is available to scroll
//       scrolling is not supported on a disabled scroll end
//       focus is not supported on a scroll end
#define XUI_SCROLLMORE              L"ScrollMore"               // more available to scroll: when not specified by the visual, try XUI_NORMAL
#define XUI_ENDSCROLLMORE           L"EndScrollMore"
#define XUI_SCROLLING               L"Scrolling"                // scrolling: when not specified by the visual, try XUI_SCROLLMORE
#define XUI_ENDSCROLLING            L"EndScrolling"

#define XUI_SCROLLMOREDISABLE       L"ScrollMoreDisable"        // when not specified by the visual, try XUI_SCROLLMORE
#define XUI_ENDSCROLLMOREDISABLE    L"EndScrollMoreDisable"

// Mouse hover related states
// States that end in Hover are optional without fallback.  If a particular state
// is not found, the existing animation playing on the control's visual continues.
// States that end in KillHover are optinal as well.  These states are run when 
// the control loses the mouse hover.  The fallback for each KillHover state is
// the same state with KillHover removed.  For example, the fallback for NormalCheckKillHover
// is NormalCheck.
#define XUI_NORMALHOVER             L"NormalHover"
#define XUI_ENDNORMALHOVER          L"EndNormalHover"
#define XUI_NORMALKILLHOVER         L"NormalKillHover"
#define XUI_ENDNORMALKILLHOVER      L"EndNormalKillHover"

#define XUI_FOCUSHOVER              L"FocusHover"
#define XUI_ENDFOCUSHOVER           L"EndFocusHover"
#define XUI_FOCUSKILLHOVER          L"FocusKillHover"
#define XUI_ENDFOCUSKILLHOVER       L"EndFocusKillHover"

#define XUI_NORMALCHECKHOVER        L"NormalCheckHover"
#define XUI_ENDNORMALCHECKHOVER     L"EndNormalCheckHover"
#define XUI_NORMALCHECKKILLHOVER    L"NormalCheckKillHover"
#define XUI_ENDNORMALCHECKKILLHOVER L"EndNormalCheckKillHover"

#define XUI_FOCUSCHECKHOVER         L"FocusCheckHover"
#define XUI_ENDFOCUSCHECKHOVER      L"EndFocusCheckHover"
#define XUI_FOCUSCHECKKILLHOVER     L"FocusCheckKillHover"
#define XUI_ENDFOCUSCHECKKILLHOVER  L"EndFocusCheckKillHover"

#define XUI_NORMALSELHOVER          L"NormalSelHover"
#define XUI_ENDNORMALSELHOVER       L"EndNormalSelHover"
#define XUI_NORMALSELKILLHOVER      L"NormalSelKillHover"
#define XUI_ENDNORMALSELKILLHOVER   L"EndNormalSelKillHover"

#define XUI_NORMALSELCHECKHOVER         L"NormalSelCheckHover"
#define XUI_ENDNORMALSELCHECKHOVER      L"EndNormalSelCheckHover"
#define XUI_NORMALSELCHECKKILLHOVER     L"NormalSelCheckKillHover"
#define XUI_ENDNORMALSELCHECKKILLHOVER  L"EndNormalSelCheckKillHover"

#define XUI_SCROLLMOREHOVER         L"ScrollMoreHover"
#define XUI_ENDSCROLLMOREHOVER      L"EndScrollMoreHover"
#define XUI_SCROLLMOREKILLHOVER     L"ScrollMoreKillHover"
#define XUI_ENDSCROLLMOREKILLHOVER  L"EndScrollMoreKillHover"

// Control visual elements: edit
#define XUI_EDITCARET               L"Caret"
#define XUI_EDITTEXT                L"Text"
#define XUI_EDITSCROLLLEFT          L"ScrollLeft"
#define XUI_EDITSCROLLRIGHT         L"ScrollRight"
#define XUI_EDITSCROLLUP            L"ScrollUp"
#define XUI_EDITSCROLLDOWN          L"ScrollDown"
#define XUI_EDITSCROLLBARV          L"ScrollBarV"
#define XUI_EDITSCROLLBARH          L"ScrollBarH"
#define XUI_EDITLANG1               L"Lang1"
#define XUI_EDITLANG2               L"Lang2"
#define XUI_EDITREADING             L"ReadingPane"

// Control visual elements: progress bars and sliders

#define XUI_PROGRESSBODY            L"ProgressBody"             // ID of visual element to be used as progress bar body
#define XUI_PROGRESSVALUE           1                           // DataAssociation for text presenters of progress value
#define XUI_SLIDERBODY              L"SliderBody"               // ID of visual element to be used as slider body
#define XUI_SLIDERVALUE             1                           // DataAssociation for text presenters of slider value
#define XUI_SLIDERTHUMB             L"SliderThumb"              // ID of visual element to be used as slider thumb
#define XUI_SLIDERSCROLLMIN         L"ScrollMin"                // ID of visual element that indicates whether slider is at min position or not
#define XUI_SLIDERSCROLLMAX         L"ScrollMax"                // ID of visual element that indicates whether slider is at max position or not

// Control visual elements: scroll bar

#define XUI_SCROLLBARBODY           L"ScrollBarBody"            // ID of visual element to be used as the scroll bar body (ie. the complete 'hit' area)
#define XUI_SCROLLBARTHUMB          L"ScrollBarThumb"           // ID of visual element to be used as the scroll bar thumb (ie. the resizable 'grab' area)

// Control visual elements: list control

#define XUI_LIST_ITEMGROUP          L"ListItemGroup"            // ID of visual element containing list items (e.g. for smooth-scroll clipping)
#define XUI_LIST_WATERMARK          L"ListWatermark"            // ID of visual element for watermark overlay (e.g. index while smooth-scrolling)

// Control visual elements: message box

#define XUI_MESSAGEBOX_BUTTON       L"Button"
#define XUI_MESSAGEBOX_MESSAGETEXT  L"MessageText"

// Scene transition namedframes
#define XUI_SCENE_TRANS_FROM        L"TransFrom"
#define XUI_SCENE_END_TRANS_FROM    L"EndTransFrom"
#define XUI_SCENE_TRANS_TO          L"TransTo"
#define XUI_SCENE_END_TRANS_TO      L"EndTransTo"
#define XUI_SCENE_TRANS_BACKTO      L"TransBackTo"
#define XUI_SCENE_END_TRANS_BACKTO  L"EndTransBackTo"
#define XUI_SCENE_TRANS_BACKFROM      L"TransBackFrom"
#define XUI_SCENE_END_TRANS_BACKFROM  L"EndTransBackFrom"

// Combo Box Classes
#define XUI_CLASS_COMBOBOX          L"XuiComboBox"
#define XUI_CLASS_COMBOBOXLIST      L"XuiComboBoxList"

// Combo Box Visual Elements
#define XUI_COMBOBOXSTYLE_DROPLIST  0
#define XUI_COMBOBOXSTYLE_DROPEDIT  1
#define XUI_COMBOBOX_VALUE          L"ComboBoxValue" // ID of visual element that displays the value of the current selection
#define XUI_COMBOBOX_LIST           L"ComboBoxList"  // ID of visual element to be used as the combo box list control
#define XUI_COMBOBOX_DROP           L"ComboBoxDrop"  // ID of visual element to be used as the combo box drop button

// XuiControl APIs: XuiListItem
enum XUI_LISTITEM_LAYOUT
{
    XUI_LISTITEM_LAYOUT_MULTI_VERTICAL,
    XUI_LISTITEM_LAYOUT_MULTI_HORIZONTAL,
    XUI_LISTITEM_LAYOUT_SINGLE_VERTICAL,
    XUI_LISTITEM_LAYOUT_SINGLE_HORIZONTAL,
    XUI_LISTITEM_LAYOUT_GRID_VERTICAL,
    XUI_LISTITEM_LAYOUT_GRID_HORIZONTAL
};

// XuiControl APIs: XuiScrollEnd
enum XUI_SCROLLEND_DIRECTION
{
    XUI_SCROLLEND_DIRECTION_UP,
    XUI_SCROLLEND_DIRECTION_DOWN,
    XUI_SCROLLEND_DIRECTION_LEFT,
    XUI_SCROLLEND_DIRECTION_RIGHT
};

// XuiControl APIs: XuiScrollBar
enum XUI_SCROLLBAR_DIRECTION
{
    XUI_SCROLLBAR_VERTICAL,
    XUI_SCROLLBAR_HORIZONTAL
};

// XuiImagePresenter mode flags
#define XUI_IMAGE_PRESENTER_MODE_NORMAL                         0x00000000
#define XUI_IMAGE_PRESENTER_MODE_AUTOSIZE                       0x00000001
#define XUI_IMAGE_PRESENTER_MODE_CENTER                         0x00000002
#define XUI_IMAGE_PRESENTER_MODE_STRETCH                        0x00000004
#define XUI_IMAGE_PRESENTER_MODE_STRETCH_MAINTAIN_ASPECT        0x00000008
#define XUI_IMAGE_PRESENTER_MODE_STRETCH_CENTER_MAINTAIN_ASPECT 0x00000010