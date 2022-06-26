#pragma once
#include <Arduino.h>
#include "Frame/FrameData.h"
#include "DeviceStatus.h"

namespace dudanov {
namespace midea {
namespace ac {

typedef unsigned char uint8_t;

class DataBodyDevOld : public FrameData {
 public:
  DataBodyDevOld() : commandType(64) {}
  explicit DataBodyDevOld(uint8_t cmdType) : commandType(cmdType) {}
  explicit DataBodyDevOld(uint8_t cmdType, const CmdB5 &cmdB5) : commandType(cmdType), mB5(cmdB5) {}
  void setDataBodyStatus(const DeviceStatus &deviceStatus) {
    this->powerStatus = deviceStatus.powerStatus;
    this->mode = deviceStatus.mode;
    this->setTemperature = deviceStatus.setTemperature;
    this->setTemperature_dot = deviceStatus.setTemperature_dot;
    this->fanSpeed = deviceStatus.fanSpeed;
    this->timer_off = deviceStatus.timer_off;
    this->timer_off_hour = deviceStatus.timer_off_hour;
    this->timer_off_minH = deviceStatus.timer_off_min / 15;
    this->timer_off_minL = deviceStatus.timer_off_min % 15;
    this->timer_on = deviceStatus.timer_on;
    this->timer_on_hour = deviceStatus.timer_on_hour;
    this->timer_on_minH = deviceStatus.timer_on_min / 15;
    this->timer_on_minL = deviceStatus.timer_on_min % 15;
    this->feelOwn = deviceStatus.feelOwn;
    this->powerSaver = deviceStatus.powerSaver;
    this->tubro = deviceStatus.tubro;
    this->lowFerqFan = deviceStatus.lowFerqFan;
    this->save = deviceStatus.save;
    this->alarmSleep = deviceStatus.alarmSleep;
    this->cosySleep = deviceStatus.cosySleep;
    this->eco = deviceStatus.eco;
    this->cleanUp = deviceStatus.cleanUp;
    this->ptcButton = deviceStatus.ptcButton;
    this->ptcAssis = deviceStatus.ptcAssis;
    this->dryClean = deviceStatus.dryClean;
    this->exchangeAir = deviceStatus.exchangeAir;
    this->wiseEye = deviceStatus.wiseEye;
    this->sleepFunc = deviceStatus.sleepFunc;
    this->catchCold = deviceStatus.catchCold;
    this->nightLight = deviceStatus.nightLight;
    this->peakElec = deviceStatus.peakElec;
    this->naturalFan = deviceStatus.naturalFan;
    this->pmvFunc_H = deviceStatus.pmvFunc_H;
    this->pmvFunc_L = deviceStatus.pmvFunc_L;
    this->leftRightFan = deviceStatus.leftRightFan;
    this->updownFan = deviceStatus.updownFan;
    this->superFan = deviceStatus.superFan;
    this->cleanFanTime = deviceStatus.cleanFanTime;
    this->tempUnit = deviceStatus.tempUnit;
    this->Eight_Hot = deviceStatus.Eight_Hot;
    this->hasNoWindFeel = deviceStatus.hasNoWindFeel;
    this->setCmd = deviceStatus.setCmd;
    this->clearCmd = deviceStatus.clearCmd;
    this->resetCmd = deviceStatus.resetCmd;
    this->addupCmd = deviceStatus.addupCmd;
    this->runmodeCmd = deviceStatus.runmodeCmd;
    this->setallusageCmd = deviceStatus.setallusageCmd;
    this->powerValue1 = deviceStatus.powerValue1;
    this->powerValue2 = deviceStatus.powerValue2;
    this->powerValue3 = deviceStatus.powerValue3;
    this->time1 = deviceStatus.time1;
    this->timeLimit = deviceStatus.timeLimit;
    this->time3 = deviceStatus.time3;
    this->humidity = deviceStatus.humidity;
    this->setNewTemperature = deviceStatus.setNewTemperature;

    if (this->powerStatus == 0) {
      this->eco = 0;
      this->tubro = 0;
    }
    uint8_t mode = this->mode;
    if (mode == 5) {
      this->eco = 0;
      this->tubro = 0;
    }
    if (mode != 3 && this->fanSpeed == 101) {
      this->fanSpeed = 102;
    }
    uint8_t power = this->powerStatus;
    if (power == 0) {
      this->eco = 0;
      this->tubro = 0;
    }
    byte[] bArr2 = new byte[25];

    this->Zerobyte = 64;
    this->FirstByte = power | 2 | (this->imodeResume & 4) | (this->childSleepMode & 8) | (this->timerMode & 16) |
                       (this->test2 & 32) | 64;
    if (this->setTemperature >= 50.0f) {
      CmdB5 cmdB5 = this->mB5;
      if (cmdB5 != null && cmdB5.isHavePoint) {
        if (String.valueOf(this->setTemperature).contains(".")) {
          str = TemperatureUtil.fahrenheit2Centigrade(String.valueOf(this->setTemperature).substring(0, 2) + ".0");
        } else {
          str = TemperatureUtil.fahrenheit2Centigrade(this->setTemperature + ".0");
        }
        if (str.contains(".5")) {
          this->setTemperature_dot = 1;
        } else {
          this->setTemperature_dot = 0;
        }
        if (str.contains(".")) {
          str = str.substring(0, 2);
        }
      } else {
        String valueOf = String.valueOf(this->setTemperature);
        if (!valueOf.contains(".")) {
          valueOf = valueOf + ".0";
        }
        str = TemperatureUtil.fahrenheit2Centigrade(valueOf);
        if (str.contains(".5")) {
          this->setTemperature_dot = 1;
        } else {
          this->setTemperature_dot = 0;
        }
      }
      this->setTemperature = Float.valueOf(str).floatValue();
    }
    this->setNewTemperature = (int) this->setTemperature;
    L.d("rawData", "288  eco = " + ((int) this->eco));
    float f = this->setTemperature - 16.0f;
    this->setTemperature = f;
    if (f < 1.0f || f > 14.0f) {
      this->setTemperature = 1.0f;
    }
    L.d("DataBodyDevOld", "setTemperature_dot = " + ((int) this->setTemperature_dot));
    L.d("rawData", "310  eco = " + ((int) this->eco));
    this->Secondbyte =
        (((this->mode << 5) & 224) | ((((int) this->setTemperature)) & 15) | ((this->setTemperature_dot << 4) & 16));
    this->Thirdbyte = (this->fanSpeed | ((this->timerEffe << 7) & 128));
    uint8_t b17 = this->timer_on;
    this->Fourthbyte = (((b17 << 7) & 128) | ((this->timer_on_hour << 2) & 124) | (this->timer_on_minH & 3));
    uint8_t b18 = this->timer_off;
    this->Fifthbyte = (((b18 << 7) & 128) | ((this->timer_off_hour << 2) & 124) | (this->timer_off_minH & 3));
    uint8_t b19 = ((this->timer_off_minL & 15) | ((this->timer_on_minL << 4) & 240));
    this->Sixthbyte = b19;
    if (((b17 << 7) & 128) == 0) {
      this->Fourthbyte = ByteCompanionObject.MAX_VALUE;
      this->Sixthbyte = (b19 & 15);
    }
    if (((b18 << 7) & 128) == 0) {
      this->Fifthbyte = ByteCompanionObject.MAX_VALUE;
      this->Sixthbyte = (this->Sixthbyte & 240);
    }
    L.d("rawData", "317  eco = " + ((int) this->eco));
    this->Seventhbyte = ((this->leftRightFan != 0 ? 3 : 0) | 48 | (this->updownFan != 0 ? 12 : 0));
    int i = (this->cosySleep & 3) | ((this->alarmSleep << 2) & 4) | ((this->save << 3) & 8) |
            ((this->lowFerqFan << 4) & 16);
    uint8_t b20 = this->tubro;
    this->Eighthbyte = (i | ((b20 << 5) & 32) | ((this->powerSaver << 6) & 64) | ((this->feelOwn << 7) & 128));
    this->Ninethbyte = ((this->wiseEye & 1) | ((this->exchangeAir << 1) & 2) | ((this->dryClean << 2) & 4) |
                        ((this->ptcAssis << 3) & 8) | ((this->ptcButton << 4) & 16) | ((this->cleanUp << 5) & 32) |
                        ((this->changeCosySleep << 6) & 64) | ((this->eco << 7) & 128));
    this->Tenthbyte = ((this->sleepFunc & 1) | ((b20 << 1) & 2) | ((this->tempUnit << 2) & 4) |
                       ((this->catchCold << 3) & 8) | ((this->nightLight << 4) & 16) | ((this->peakElec << 5) & 32) |
                       ((this->dusFull << 6) & 64) | ((this->cleanFanTime << 7) & 128));
    L.d("catchCold", "发送  " + ((int) this->catchCold));
    L.d("ApplianceStatus", "Tenthbyte " + ((int) this->Tenthbyte) + "   " + ((int) this->cleanFanTime));
    this->Eleventhbyte = 0;
    this->Twelfthbyte = 0;
    this->Thirteenthbyte = 0;
    this->Fourteenthbyte = 0;
    this->Fifteenthbyte = ((this->naturalFan << 6) & 64);
    this->Sixteenthbyte = 0;
    this->Seventeenthbyte = 0;
    L.d("DataBodyDevOld",
        "setNewTemperature = " + this->setNewTemperature + " --- -- setTemperature =" + this->setTemperature);
    this->Eighteenthbyte = ((((this->setNewTemperature - 12) & 31)) | 0);
    this->Nineteenthbyte = (this->humidity | 0);
    this->Twentieth = 0;
    this->Twenty_first = ((this->setExpand_dot & 1) | ((this->setExpand << 1) & 62) | ((this->double_temp << 6) & 64) |
                          ((this->Eight_Hot << 7) & 128));
    updateProtocol(getProtocol());
    setBytes(bArr2);
    byte[] addHead = addHead(bArr2);
    L.d("DataBodyDevOld", "FirstByte = " + ((int) this->FirstByte));
    L.d("DataBodyDevOld", "Secondbyte = " + ((int) this->Secondbyte));
    L.i("yun", "send : " + Arrays.toString(Utils.parseBytes2HexStr(addHead)));
    return addHead;
  }

  uint8_t getEco() const { return this->eco; }

  CmdB5 mB5;
  float setTemperature;
  int setNewTemperature;
  uint8_t Eight_Hot;
  uint8_t alarmSleep;
  uint8_t catchCold;
  uint8_t changeCosySleep;
  uint8_t childSleepMode;
  uint8_t cleanFanTime;
  uint8_t cleanUp;
  uint8_t controlSrc{1};
  uint8_t cosySleep;
  uint8_t double_temp;
  uint8_t dryClean;
  uint8_t dusFull;
  uint8_t eco;
  uint8_t exchangeAir;
  uint8_t fanSpeed;
  uint8_t feelOwn;
  uint8_t hasNoWindFeel;
  uint8_t humidity;
  uint8_t imodeResume;
  uint8_t keyBtn;
  uint8_t keyStatus{1};
  uint8_t leftRightFan;
  uint8_t lowFerqFan;
  uint8_t mode;
  uint8_t naturalFan;
  uint8_t nightLight;
  uint8_t peakElec;
  uint8_t pmvFunc_H;
  uint8_t pmvFunc_L;
  uint8_t powerSaver;
  uint8_t powerStatus;
  uint8_t ptcAssis;
  uint8_t ptcButton;
  uint8_t save;
  uint8_t setExpand;
  uint8_t setExpand_dot;
  uint8_t setTemperature_dot;
  uint8_t sleepFunc;
  uint8_t superFan;
  uint8_t tempUnit;
  uint8_t test2;
  uint8_t timerEffe;
  uint8_t timerMode;
  uint8_t timer_off;
  uint8_t timer_off_hour;
  uint8_t timer_off_minH;
  uint8_t timer_off_minL;
  uint8_t timer_on;
  uint8_t timer_on_hour;
  uint8_t timer_on_minH;
  uint8_t timer_on_minL;
  uint8_t tubro;
  uint8_t updownFan;
  uint8_t wiseEye;

 protected:
  static uint8_t number = 4;
  uint8_t addupCmd;
  uint8_t clearCmd;
  uint8_t commandType;
  uint8_t powerUsage{4};
  uint8_t powerValue1;
  uint8_t powerValue2;
  uint8_t powerValue3;
  uint8_t readCmd{64};
  uint8_t resetCmd;
  uint8_t runmodeCmd;
  uint8_t setCmd;
  uint8_t setallusageCmd;
  uint8_t time1;
  uint8_t time3;
  uint8_t timeLimit;
  uint8_t writeCmd{0};
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
