#include "KBRun.hh"
#include "KBChannelHit.hh"
#include "DUMMYDoSomethingTask.hh"

ClassImp(DUMMYDoSomethingTask)

DUMMYDoSomethingTask::DUMMYDoSomethingTask()
:KBTask("DUMMYDoSomethingTask","")
{
}

bool DUMMYDoSomethingTask::Init()
{
  bool saveThisBranch = true;
  fChannelArray = new TClonesArray("KBChannelHit", 100);
  run -> RegisterBranch("Channelhit", fChannelArray, saveThisBranch);

  return true;
}

void DUMMYDoSomethingTask::Exec(Option_t*)
{
  fChannelArray -> Clear();

  auto hit = (KBHit *) fChannelArray -> ConstructedAt(0);
  hit -> SetID(0);
  hit -> SetTDC(0);
  hit -> SetADC(123.987);

  auto hit = (KBHit *) fChannelArray -> ConstructedAt(1);
  hit -> SetID(1);
  hit -> SetTDC(1.1);
  hit -> SetADC(1000);

  auto hit = (KBHit *) fChannelArray -> ConstructedAt(2);
  hit -> SetID(2);
  hit -> SetTDC(22);
  hit -> SetADC(8);

  cout << "  [" << this -> GetName() << "] Message from this task" << endl;
}
