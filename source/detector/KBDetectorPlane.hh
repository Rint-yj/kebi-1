#ifndef KBDETECTORPLANE_HH
#define KBDETECTORPLANE_HH

#include "KBChannel.hh"
#include "KBParameterContainerHolder.hh"

#include "TH2.h"
#include "TCanvas.h"
#include "TObject.h"
#include "TNamed.h"
#include "TObjArray.h"
#include "TClonesArray.h"

class KBDetectorPlane : public TNamed, public KBParameterContainerHolder
{
  public:
    KBDetectorPlane(const char *name, const char *title);
    virtual ~KBDetectorPlane() {};

    virtual void Clear(Option_t *option = "");
    virtual void Print(Option_t *option = "") const;
    virtual bool Init() = 0;

    virtual bool IsInBoundary(Double_t i, Double_t j) = 0;

    virtual Int_t FindChannelID(Double_t i, Double_t j) = 0;

    virtual TCanvas *GetCanvas(Option_t *option = "");
    virtual void DrawFrame(Option_t *option = "");
    virtual TH2* GetHist(Option_t *option = "-1") = 0;

  public:
    void SetPlaneID(Int_t id);
    Int_t GetPlaneID() const;

    void SetPlaneK(Double_t k);
    Double_t GetPlaneK();

    KBChannel *GetChannelFast(Int_t idx);
    KBChannel *GetChannel(Int_t idx);

    Int_t GetNChannels();
    TObjArray *GetChannelArray();

  protected:
    TObjArray *fChannelArray = nullptr;

    Int_t fPlaneID = -1;
    Double_t fPlaneK = -999;

    TCanvas *fCanvas = nullptr;
    TH2 *fH2Plane = nullptr;


  ClassDef(KBDetectorPlane, 1)
};

#endif
