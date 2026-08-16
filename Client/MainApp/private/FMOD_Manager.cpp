#include "Client_pch.h"
#include "FMOD_Manager.h"

FMOD_Manager* FMOD_Manager::m_pInstance = nullptr;

// ===== Helpers =====
static inline bool FMOD_OK_CHECK(FMOD_RESULT r) { return r == FMOD_OK; }
static inline void FMOD_LOG(const char* what, FMOD_RESULT r)
{
#ifdef _DEBUG
    if (!FMOD_OK_CHECK(r)) {
        std::string s = std::string("[FMOD] ") + what + " : " + FMOD_ErrorString(r) + "\n";
        OutputDebugStringA(s.c_str());
    }
#endif
}

static inline FMOD_VECTOR ToFVec(const AudioVec3& v) { FMOD_VECTOR r{ v.x, v.y, v.z }; return r; }

static FMOD_MODE RolloffToFMOD(AudioRolloff m)
{
    switch (m) {
    case AudioRolloff::Inverse:        return FMOD_3D_INVERSEROLLOFF;
    case AudioRolloff::Linear:         return FMOD_3D_LINEARROLLOFF;
    case AudioRolloff::LinearSquared:  return FMOD_3D_LINEARSQUAREROLLOFF;
    case AudioRolloff::InverseTapered: return FMOD_3D_INVERSETAPEREDROLLOFF;
    case AudioRolloff::Custom:         return FMOD_3D_CUSTOMROLLOFF;
    default:                           return FMOD_3D_INVERSEROLLOFF;
    }
}

// ===== Life-cycle =====
bool FMOD_Manager::Initialize(int maxChannels, float distanceScale)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mSystem) return true;

    FMOD_RESULT r = FMOD::System_Create(&mSystem);
    FMOD_LOG("System_Create", r);
    if (!FMOD_OK_CHECK(r) || !mSystem) return false;

    r = mSystem->init(maxChannels, 0, nullptr);
    FMOD_LOG("System::init", r);
    if (!FMOD_OK_CHECK(r)) return false;

    mDistanceScale = distanceScale;
    mSystem->set3DSettings(/*doppler*/1.0f, /*distance*/mDistanceScale, /*rolloff*/1.0f);

    r = mSystem->getMasterChannelGroup(&mMaster);
    FMOD_LOG("System::getMasterChannelGroup", r);
    return FMOD_OK_CHECK(r);
}

void FMOD_Manager::Update()
{
    if (mSystem) mSystem->update();
}

void FMOD_Manager::Shutdown()
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mSystem) return;

    UnloadAll();

    mSystem->close();
    mSystem->release();
    mSystem = nullptr;
    mMaster = nullptr;
}

// ===== Sound Assets =====
bool FMOD_Manager::LoadSound(const std::string& id, const char* path, bool is3D, bool loop, bool stream)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mSystem) return false;
    if (mSounds.count(id)) return true;

    FMOD_MODE mode = FMOD_DEFAULT | (is3D ? FMOD_3D : FMOD_2D) | (loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
    if (stream) mode |= FMOD_CREATESTREAM;

    FMOD::Sound* s = nullptr;
    FMOD_RESULT r = mSystem->createSound(path, mode, nullptr, &s);
    FMOD_LOG(("createSound " + id).c_str(), r);
    if (!FMOD_OK_CHECK(r) || !s) return false;

    mSounds[id] = s;
    return true;
}

void FMOD_Manager::UnloadSound(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mSounds.find(id);
    if (it != mSounds.end()) {
        if (it->second) it->second->release();
        mSounds.erase(it);
    }
}

void FMOD_Manager::UnloadAll()
{
    for (auto& kv : mSounds) {
        if (kv.second) kv.second->release();
    }
    mSounds.clear();
}

bool FMOD_Manager::SetSound3DDistance(const std::string& id, float minDistance, float maxDistance)
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mSounds.find(id);
    if (it == mSounds.end() || !it->second) return false;

    FMOD_RESULT r = it->second->set3DMinMaxDistance(minDistance, maxDistance);
    FMOD_LOG(("set3DMinMaxDistance " + id).c_str(), r);
    return FMOD_OK_CHECK(r);
}

bool FMOD_Manager::SetSound3DRolloff(const std::string& id, AudioRolloff rolloff)
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mSounds.find(id);
    if (it == mSounds.end() || !it->second) return false;

    FMOD_MODE cur = FMOD_DEFAULT;
    it->second->getMode(&cur);

    // 기존 롤오프 플래그 제거
    cur &= ~(FMOD_3D_INVERSEROLLOFF | FMOD_3D_LINEARROLLOFF | FMOD_3D_LINEARSQUAREROLLOFF |
        FMOD_3D_INVERSETAPEREDROLLOFF | FMOD_3D_CUSTOMROLLOFF);

    cur |= RolloffToFMOD(rolloff);

    FMOD_RESULT r = it->second->setMode(cur);
    FMOD_LOG(("setMode rolloff " + id).c_str(), r);
    return FMOD_OK_CHECK(r);
}

void FMOD_Manager::Set3DSettings(float dopplerScale, float distanceScale, float rolloffScale)
{
    mDistanceScale = distanceScale;
    if (mSystem) mSystem->set3DSettings(dopplerScale, distanceScale, rolloffScale);
}

// ===== Play =====
bool FMOD_Manager::Play2D(const std::string& id, float volume, bool paused)
{
    FMOD::Channel* ch = nullptr;
    return Play2D_ReturnChannel(id, &ch, volume, paused);
}

bool FMOD_Manager::Play3D(const std::string& id, const AudioVec3& pos, const AudioVec3& vel, float volume, bool paused)
{
    FMOD::Channel* ch = nullptr;
    return Play3D_ReturnChannel(id, pos, vel, &ch, volume, paused);
}

bool FMOD_Manager::Play2D_ReturnChannel(const std::string& id, FMOD::Channel** outCh, float volume, bool paused)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mSystem) return false;
    auto it = mSounds.find(id); if (it == mSounds.end()) return false;

    FMOD_RESULT r = mSystem->playSound(it->second, mMaster, paused, outCh);
    FMOD_LOG(("playSound2D " + id).c_str(), r);
    if (!FMOD_OK_CHECK(r) || !(*outCh)) return false;

    (*outCh)->setVolume(volume);
    return true;
}

bool FMOD_Manager::Play3D_ReturnChannel(const std::string& id, const AudioVec3& pos, const AudioVec3& vel,
    FMOD::Channel** outCh, float volume, bool paused)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mSystem) return false;
    auto it = mSounds.find(id); if (it == mSounds.end()) return false;

    // 3D 속성 세팅 위해 일단 pause로 받기
    FMOD_RESULT r = mSystem->playSound(it->second, mMaster, true, outCh);
    FMOD_LOG(("playSound3D " + id).c_str(), r);
    if (!FMOD_OK_CHECK(r) || !(*outCh)) return false;

    FMOD_VECTOR fp = ToFVec(pos);
    FMOD_VECTOR fv = ToFVec(vel);
    (*outCh)->set3DAttributes(&fp, &fv);
    (*outCh)->setVolume(volume);
    (*outCh)->setPaused(paused);
    return true;
}

// ===== Listener =====
void FMOD_Manager::SetListener(const AudioVec3& pos, const AudioVec3& vel, const AudioVec3& forward, const AudioVec3& up)
{
    if (!mSystem) return;
    FMOD_VECTOR p = ToFVec(pos);
    FMOD_VECTOR v = ToFVec(vel);
    FMOD_VECTOR f = ToFVec(forward);
    FMOD_VECTOR u = ToFVec(up);
    mSystem->set3DListenerAttributes(0, &p, &v, &f, &u);
}

void FMOD_Manager::SetListener(const FMOD_VECTOR& pos,
    const FMOD_VECTOR& vel,
    const FMOD_VECTOR& forward,
    const FMOD_VECTOR& up)
{
    if (!mSystem) return;
    mSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
}

void FMOD_Manager::SetMasterVolume(float v)
{
    if (mMaster) mMaster->setVolume(v);
}