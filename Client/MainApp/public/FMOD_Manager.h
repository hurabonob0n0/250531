#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <cstdint>

// ---- FMOD 전방 선언 (헤더 include 없이 포인터/참조만 사용) ----
namespace FMOD {
    class System;
    class ChannelGroup;
    class Channel;
    class Sound;
    struct VECTOR;
}

// 우리 쪽에서 쓰는 3D 벡터
struct AudioVec3 {
    float x, y, z;
    AudioVec3(float X = 0, float Y = 0, float Z = 0) : x(X), y(Y), z(Z) {}
};

// FMOD 롤오프 모드에 대한 중립 래퍼(enum)
enum class AudioRolloff {
    Inverse,        // FMOD_3D_INVERSEROLLOFF
    Linear,         // FMOD_3D_LINEARROLLOFF
    LinearSquared,  // FMOD_3D_LINEARSQUAREROLLOFF
    InverseTapered, // FMOD_3D_INVERSETAPEREDROLLOFF
    Custom          // FMOD_3D_CUSTOMROLLOFF
};

class FMOD_Manager
{
private:
    FMOD_Manager() = default;
    ~FMOD_Manager() = default;

public:
    // ===== Singleton =====
    static FMOD_Manager* Get_Instance()
    {
        if (!m_pInstance) m_pInstance = new FMOD_Manager();
        return m_pInstance;
    }
    static void Destroy_Instance()
    {
        if (m_pInstance) {
            m_pInstance->Shutdown();
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    }

public:
    // ===== Life-cycle =====
    bool Initialize(int maxChannels = 512, float distanceScale = 1.0f);
    void Update();      // ★ 매 프레임 호출
    void Shutdown();

    // ===== Sound Assets =====
    bool LoadSound(const std::string& id, const char* path, bool is3D = false, bool loop = false, bool stream = false);
    void UnloadSound(const std::string& id);
    void UnloadAll();

    bool SetSound3DDistance(const std::string& id, float minDistance, float maxDistance);
    bool SetSound3DRolloff(const std::string& id, AudioRolloff rolloff);
    void Set3DSettings(float dopplerScale, float distanceScale, float rolloffScale);

    // ===== Play =====
    bool Play2D(const std::string& id, float volume = 1.0f, bool paused = false);
    bool Play3D(const std::string& id, const AudioVec3& pos, const AudioVec3& vel = AudioVec3{}, float volume = 1.0f, bool paused = false);

    // 채널 핸들이 필요하면 반환(포인터 그대로 노출하지만 전방선언로 헤더 의존성 제거)
    bool Play2D_ReturnChannel(const std::string& id, FMOD::Channel** outCh, float volume = 1.0f, bool paused = false);
    bool Play3D_ReturnChannel(const std::string& id, const AudioVec3& pos, const AudioVec3& vel,
        FMOD::Channel** outCh, float volume = 1.0f, bool paused = false);

    // 리스너(카메라)
    void SetListener(const AudioVec3& pos, const AudioVec3& vel, const AudioVec3& forward, const AudioVec3& up);
    void SetListener(const FMOD_VECTOR& pos,
        const FMOD_VECTOR& vel,
        const FMOD_VECTOR& forward,
        const FMOD_VECTOR& up);
    // 마스터 볼륨
    void SetMasterVolume(float v);

    // 필요시 시스템 접근
    FMOD::System* GetSystem() const { return mSystem; }

private:
    static FMOD_Manager* m_pInstance;

private:
    FMOD::System* mSystem = nullptr;
    FMOD::ChannelGroup* mMaster = nullptr;

    std::unordered_map<std::string, FMOD::Sound*> mSounds;
    std::mutex mMutex;

    float mDistanceScale = 1.0f;
};