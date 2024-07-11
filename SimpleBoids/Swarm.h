#pragma once

#include "Boid.h"
#include "RandomNumberHelper.h"

#include <functional>
#include <tuple>

class Swarm
{
public:
    Swarm(
        float boidRadius, 
        float boidMinDistance, 
        float boidMatchingFactor, 
        float maxBoidSpeed,
        float boidMoveToCenterFactor);

    // Creates or destroys the given number of boids.
    void AddBoids(int count);
    void RemoveBoids(int count);

    // Moves all the boids to new positions.
    void Update(float timeDelta);

    // Moves boids to initial random positions.
    void ResetBoids();

    // Iterates over all boids executing a given function.
    void Iterate(std::function<void(DirectX::XMMATRIX)> function);

    // Getters
    size_t Size() const { return m_boids.size(); }
    float GetBoidMinDistance() const { return m_boidMinDistance; }
    float GetBoidMatchingFactor() const { return m_boidMatchingFactor; }
    float GetMaxBoidSpeed() const { return m_maxBoidSpeed; }
    float GetBoidMoveToCenterFactor() const { return m_boidMoveToCenterFactor; }

    // Setters
    void SetBoidMinDistance(float boidMinDistance) { m_boidMinDistance = m_boidRadius + boidMinDistance; }
    void SetBoidMatchingFactor(float boidMatchingFactor) { m_boidMatchingFactor = boidMatchingFactor; }
    void SetMaxBoidSpeed(float maxBoidSpeed);
    void SetBoidMoveToCenterFactor(float boidMoveToCenterFactor) { m_boidMoveToCenterFactor = boidMoveToCenterFactor; }

private:
    Concurrency::critical_section               m_criticalSection;
    std::vector<std::unique_ptr<Boid>>          m_boids;
    std::unique_ptr<RandomNumberHelper>         m_rand;
    float                                       m_boidRadius;
    float                                       m_boidMinDistance; 
    float                                       m_boidMatchingFactor;
    float                                       m_maxBoidSpeed;
    float                                       m_boidMoveToCenterFactor;

    DirectX::XMVECTOR ExecuteRule1(int boidIndex);
    DirectX::XMVECTOR ExecuteRule2(int boidIndex);
    DirectX::XMVECTOR ExecuteRule3(int boidIndex, bool allBoids = true);
    DirectX::XMVECTOR ExecuteRule4(int boidIndex);

    std::tuple<DirectX::XMVECTOR, DirectX::XMVECTOR> GetRandomPositionAndVelocity();
};

