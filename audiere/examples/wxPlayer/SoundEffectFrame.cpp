#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "Commands.h"
#include "SoundEffectFrame.h"


BEGIN_EVENT_TABLE(SoundEffectFrame, wxMDIChildFrame)
  EVT_BUTTON(EFFECT_PLAY,  SoundEffectFrame::OnPlay)
  EVT_BUTTON(EFFECT_STOP,  SoundEffectFrame::OnStop)

  EVT_COMMAND_SCROLL(STREAM_VOLUME, SoundEffectFrame::OnChangeVolume)
  EVT_COMMAND_SCROLL(STREAM_PAN,    SoundEffectFrame::OnChangePan)
  EVT_COMMAND_SCROLL(STREAM_PITCH,  SoundEffectFrame::OnChangePitch)
END_EVENT_TABLE()


SoundEffectFrame::SoundEffectFrame(wxMDIParentFrame* parent, const wxString& title, audiere::SoundEffect* effect)
: wxMDIChildFrame(parent, -1, title)
{
  m_effect = effect;

  m_vpp_label = new wxStaticText(this, -1, "Volume - Pan - Pitch");
  m_volume = new wxSlider(this, STREAM_VOLUME, 100, 0,  100);
  m_pan    = new wxSlider(this, STREAM_PAN,    0, -100, 100);
  m_pitch  = new wxSlider(this, STREAM_PITCH,  100, 50, 200);

  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(
    new wxButton(this, EFFECT_PLAY, "Play"),
    1, wxEXPAND | wxALL, 4);
  sizer->Add(
    new wxButton(this, EFFECT_STOP, "Stop"),
    1, wxEXPAND | wxALL, 4);
  sizer->Add(m_vpp_label, 1, wxEXPAND | wxALL, 4);
  sizer->Add(m_volume,    1, wxEXPAND | wxALL, 4);
  sizer->Add(m_pan,       1, wxEXPAND | wxALL, 4);
  sizer->Add(m_pitch,     1, wxEXPAND | wxALL, 4);

  UpdateVPPLabel();

  SetAutoLayout(true);
  SetSizer(sizer);

  sizer->Fit(this);
  sizer->SetSizeHints(this);
}


void SoundEffectFrame::OnPlay() {
  m_effect->play();
}


void SoundEffectFrame::OnStop() {
  m_effect->stop();
}


void SoundEffectFrame::OnChangeVolume() {
  m_effect->setVolume(m_volume->GetValue() / 100.0f);
  UpdateVPPLabel();
}


void SoundEffectFrame::OnChangePan() {
  m_effect->setPan(m_pan->GetValue() / 100.0f);
  UpdateVPPLabel();
}


void SoundEffectFrame::OnChangePitch() {
  m_effect->setPitchShift(m_pitch->GetValue() / 100.0f);
  UpdateVPPLabel();
}


void SoundEffectFrame::UpdateVPPLabel() {
  wxString label;
  label.Printf(
    "Vol: %1.2f  Pan: %1.2f  Pitch: %1.2f",
    m_volume->GetValue() / 100.0f,
    m_pan->GetValue() / 100.0f,
    m_pitch->GetValue() / 100.0f);
  m_vpp_label->SetLabel(label);
}
