#include "KBPadPlane.hh"

#include <iostream>
using namespace std;

ClassImp(KBPadPlane)

KBPadPlane::KBPadPlane(const char *name, const char *title)
:KBDetectorPlane(name, title)
{
}

void KBPadPlane::Print(Option_t *) const
{
  cout << "Pad Plane containing " << fChannelArray -> GetEntries() << " pads" << endl;
}

KBPad *KBPadPlane::GetPadFast(Int_t idx) { return (KBPad *) fChannelArray -> At(idx); }

KBPad *KBPadPlane::GetPad(Int_t idx)
{
  TObject *obj = nullptr;
  if (idx >= 0 && idx < fChannelArray -> GetEntriesFast())
    obj = fChannelArray -> At(idx);

  return (KBPad *) obj;
}

void KBPadPlane::SetPadArray(TClonesArray *padArray)
{
  TIter iterPads(padArray);
  KBPad *padWithData;
  while ((padWithData = (KBPad *) iterPads.Next())) {
    if (padWithData -> GetPlaneID() != fPlaneID)
      continue;
    auto padID = padWithData -> GetPadID();
    if (padID < 0)
      continue;
    auto pad = GetPadFast(padID);
    pad -> CopyPadData(padWithData);
    pad -> SetActive();
  }
}

void KBPadPlane::SetHitArray(TClonesArray *hitArray)
{
  TIter iterPads(hitArray);
  KBHit *hit;
  while ((hit = (KBHit *) iterPads.Next())) {
    auto padID = hit -> GetPadID();
    if (padID < 0)
      continue;
    auto pad = GetPadFast(padID);
    pad -> AddHit(hit);
    pad -> SetActive();
  }
}

void KBPadPlane::FillBufferIn(Double_t i, Double_t j, Double_t tb, Double_t val)
{
  Int_t id = FindPadID(i, j);
  if (id < 0)
    return; 

  KBPad *pad = (KBPad *) fChannelArray -> At(id);
  if (pad != nullptr)
    pad -> FillBufferIn(tb, val);
}

void KBPadPlane::FillDataToHist(Option_t *option)
{
  if (fH2Plane == nullptr) {
    cout << "[KBPadPlane] Pad plane histogram does not exist! Run GetHist(option) before filling buffer." << endl;
    return;
  }

  TString optionString = TString(option);

  cout << "[KBPadPlane] " << "Filling " << optionString << " into pad-plane histogram" << endl;

  KBPad *pad;
  TIter iterPads(fChannelArray);

  if (optionString == "hit") {
    while ((pad = (KBPad *) iterPads.Next())) {
      if (pad -> GetNumHits() == 0) continue;
      auto charge = 0.;
      for (auto iHit = 0; iHit < pad -> GetNumHits(); ++iHit) {
        auto hit = pad -> GetHit(iHit);
        if (charge < hit -> GetCharge())
          charge = hit -> GetCharge();
      }
      fH2Plane -> Fill(pad->GetI(),pad->GetJ(),charge);
    }
  } else if (optionString == "out") {
    while ((pad = (KBPad *) iterPads.Next())) {
      auto buffer = pad -> GetBufferOut();
      Double_t val = *max_element(buffer,buffer+512);
      if (val < 1) val = 0;
      fH2Plane -> Fill(pad->GetI(),pad->GetJ(),val);
    }
  } else if (optionString == "raw") {
    while ((pad = (KBPad *) iterPads.Next())) {
      auto buffer = pad -> GetBufferRaw();
      Double_t val = *max_element(buffer,buffer+512);
      if (val < 1) val = 0;
      fH2Plane -> Fill(pad->GetI(),pad->GetJ(),val);
    }
  } else if (optionString == "in") {
    while ((pad = (KBPad *) iterPads.Next())) {
      auto buffer = pad -> GetBufferIn();
      Double_t val = *max_element(buffer,buffer+512);
      if (val < 1) val = 0;
      fH2Plane -> Fill(pad->GetI(),pad->GetJ(),val);
    }
  }
}

Int_t KBPadPlane::GetNPads() { return GetNChannels(); }

void KBPadPlane::SetPlaneK(Double_t k) { fPlaneK = k; }
Double_t KBPadPlane::GetPlaneK() { return fPlaneK; }

void KBPadPlane::Clear(Option_t *)
{
  KBPad *pad;
  TIter iterPads(fChannelArray);
  while ((pad = (KBPad *) iterPads.Next())) {
    pad -> Clear();
  }
}

Int_t KBPadPlane::FindChannelID(Double_t i, Double_t j) { return FindPadID(i,j); }

void KBPadPlane::ResetHitMap()
{
  KBPad *pad;
  TIter iterPads(fChannelArray);
  while ((pad = (KBPad *) iterPads.Next())) {
    pad -> ClearHits();
    pad -> LetGo();
  }

  fFreePadIdx = 0;
}

void KBPadPlane::AddHit(KBHit *hit)
{
  auto pad = (KBPad *) fChannelArray -> At(hit -> GetPadID());
  pad -> AddHit(hit);
}

KBHit *KBPadPlane::PullOutNextFreeHit()
{
  if (fFreePadIdx == fChannelArray -> GetEntriesFast() - 1)
    return nullptr;

  auto pad = (KBPad *) fChannelArray -> At(fFreePadIdx);
  auto hit = pad -> PullOutNextFreeHit();
  if (hit == nullptr) {
    fFreePadIdx++;
    return PullOutNextFreeHit();
  }

  return hit;
}

void KBPadPlane::PullOutNeighborHits(vector<KBHit*> *hits, vector<KBHit*> *neighborHits)
{
  for (auto hit : *hits) {
    auto pad = (KBPad *) fChannelArray -> At(hit -> GetPadID());
    auto neighbors = pad -> GetNeighborPadArray();
    for (auto neighbor : *neighbors)
      neighbor -> PullOutHits(neighborHits);
  }
}

void KBPadPlane::PullOutNeighborHits(TVector3 p, Int_t range, vector<KBHit*> *neighborHits)
{
  vector<KBPad *> neighborsUsed;
  vector<KBPad *> neighborsTemp;
  vector<KBPad *> neighborsNew;

  Int_t id = FindPadID(p.X(), p.Y());
  if (id < 0)
    return;

  auto pad = (KBPad *) fChannelArray -> At(id);

  neighborsTemp.push_back(pad);
  pad -> Grab();

  while (range >= 0) {
    neighborsNew.clear();
    GrabNeighborPads(&neighborsTemp, &neighborsNew);

    for (auto neighbor : neighborsTemp)
      neighborsUsed.push_back(neighbor);
    neighborsTemp.clear();

    for (auto neighbor : neighborsNew) {
      neighbor -> PullOutHits(neighborHits);
      neighborsTemp.push_back(neighbor);
    }
    range--;
  }

  for (auto neighbor : neighborsUsed)
    neighbor -> LetGo();

  for (auto neighbor : neighborsNew)
    neighbor -> LetGo();
}

void KBPadPlane::GrabNeighborPads(vector<KBPad*> *pads, vector<KBPad*> *neighborPads)
{
  for (auto pad : *pads) {
    auto neighbors = pad -> GetNeighborPadArray();
    for (auto neighbor : *neighbors) {
      if (neighbor -> IsGrabed())
        continue;
      neighborPads -> push_back(neighbor);
      neighbor -> Grab();
    }
  }
}

TObjArray *KBPadPlane::GetPadArray() { return fChannelArray; }
