#include "pch.h"

#include "Constants.h"
#include "Swarm.h"

using namespace Concurrency;
using namespace DirectX;

Swarm::Swarm(
    float boidRadius, 
    float boidMinDistance, 
    float boidMatchingFactor, 
    float maxBoidSpeed) :
    m_boidRadius(boidRadius),
    m_boidMinDistance(boidRadius + boidMinDistance),
    m_boidMatchingFactor(boidMatchingFactor),
    m_maxBoidSpeed(maxBoidSpeed)
{
    m_rand = std::make_unique<RandomNumberHelper>();
}

void Swarm::AddBoids(int count)
{
    critical_section::scoped_lock lock(m_criticalSection);

    for (auto i = 0; i < count; ++i)
    {
        auto [randomPosition, randomVelocity] = GetRandomPositionAndVelocity();
        m_boids.emplace_back(std::make_unique<Boid>(randomPosition, randomVelocity, m_maxBoidSpeed));
    }
}

void Swarm::RemoveBoids(int count)
{
    critical_section::scoped_lock lock(m_criticalSection);

    if (count >= Size())
        m_boids.clear();
    else
    {
        auto end = std::end(m_boids);
        m_boids.erase(end - count, end);
    }
}

void Swarm::Update(float timeDelta)
{
    critical_section::scoped_lock lock(m_criticalSection);

    for (int i = 0; i < Size(); ++i)
    {
        // Perform vector operations on the positions of the boids. Operations are independent from each other.

        // Rule 1: Make boids fly towards the centre of the mass of neighbouring boids.
        XMVECTOR v1 = ExecuteRule1(i);

        // Rule 2: Move away from other boids that are too close to avoid colliding.
        XMVECTOR v2 = ExecuteRule2(i);

        // Rule 3: Find the average velocity (speed and direction) of the other boids and adjust velocity slightly to match.
        XMVECTOR v3 = ExecuteRule3(i, true);

        // Rule 4: Encourage boids to stay within rough boundaries.
        XMVECTOR v4 = ExecuteRule4(i);

        XMVECTOR velocityDelta = timeDelta * (v1 + v2 + v3 + v4);

        m_boids[i]->Update(velocityDelta);
    }
}

void Swarm::ResetBoids()
{
    critical_section::scoped_lock lock(m_criticalSection);

    for (auto i = 0; i < Size(); ++i)
    {
        auto [randomPosition, randomVelocity] = GetRandomPositionAndVelocity();
        m_boids[i]->SetPosition(randomPosition);
        m_boids[i]->SetVelocity(randomVelocity);
    }
}

void Swarm::Iterate(std::function<void(DirectX::XMMATRIX)> function)
{
    critical_section::scoped_lock lock(m_criticalSection);

    for (auto i = 0; i < Size(); ++i)
    {
        function(m_boids[i]->GetWorldMatrix());
    }
}

void Swarm::SetMaxBoidSpeed(float maxBoidSpeed)
{ 
    m_maxBoidSpeed = maxBoidSpeed;

    // Update max speed of all boids in the swarm.
    for (auto i = 0; i < Size(); ++i)
    {
        m_boids[i]->SetMaxSpeed(m_maxBoidSpeed);
    }
}

DirectX::XMVECTOR Swarm::ExecuteRule1(int boidIndex)
{
    // Move the boid toward its 'perceived centre', which is the centre of all the other boids, not including itself.
    XMVECTOR sum{ XMVectorZero() };

    for (int i = 0; i < Size(); ++i)
    {
        if (boidIndex != i)
            sum = XMVectorAdd(sum, m_boids[i]->GetPosition());
    }

    size_t boidCount = Size() - 1; // all the boids minus the current boid
    XMVECTOR centre = sum / (static_cast<float>(boidCount));
    XMVECTOR boidPosition = m_boids[boidIndex]->GetPosition();
    XMVECTOR v = XMVectorSubtract(centre, boidPosition) * BOID_MOVE_TO_CENTER_FACTOR;

    return v;
}

DirectX::XMVECTOR Swarm::ExecuteRule2(int boidIndex)
{
    XMVECTOR moveDelta{ XMVectorZero() };

    for (int i = 0; i < Size(); ++i)
    {
        if (boidIndex != i)
        {
            // Calculate the distance of this boid to the other boid.
            auto p1 = m_boids[boidIndex]->GetPosition();
            auto p2 = m_boids[i]->GetPosition();
            auto diff = XMVectorSubtract(p2, p1);
            auto distance = XMVectorGetX(XMVector3Length(diff));

            // Accumulate the displacement of each boid that is nearby.
            if (distance < m_boidMinDistance)
                moveDelta = XMVectorSubtract(moveDelta, diff);
        }
    }

    XMVECTOR v = BOID_AVOID_FACTOR * moveDelta;
    return v;
}

DirectX::XMVECTOR Swarm::ExecuteRule3(int boidIndex, bool allBoids)
{
    // Method #1: Take into account all boids.
    if (allBoids)
    {
        XMVECTOR avg{ XMVectorZero() };

        for (int i = 0; i < Size(); ++i)
        {
            if (boidIndex != i)
                avg = XMVectorAdd(avg, m_boids[i]->GetVelocity());
        }

        size_t boidCount = Size() - 1; // all the boids minus the current boid
        XMVECTOR centre = avg / static_cast<float>(boidCount);
        XMVECTOR boidVelocity = m_boids[boidIndex]->GetVelocity();
        XMVECTOR v = XMVectorSubtract(centre, boidVelocity) * m_boidMatchingFactor;

        return v;
    }
    // Method #2: Take into account only boids in a certain range from a given boid.
    else
    {
        XMVECTOR avg{ XMVectorZero() };
        int neighborCount = 0;

        for (int i = 0; i < Size(); ++i)
        {
            if (boidIndex != i)
            {
                // Calculate the distance of this boid to the other boid.
                auto p1 = m_boids[boidIndex]->GetPosition();
                auto p2 = m_boids[i]->GetPosition();
                auto diff = XMVectorSubtract(p2, p1);
                auto distance = XMVectorGetX(XMVector3Length(diff));

                if (distance < BOID_VISUAL_RANGE)
                {
                    avg = XMVectorAdd(avg, m_boids[i]->GetVelocity());
                    ++neighborCount;
                }
            }
        }

        XMVECTOR v = XMVectorZero();

        if (neighborCount > 0)
        {
            XMVECTOR centre = avg / static_cast<float>(neighborCount);
            XMVECTOR boidVelocity = m_boids[boidIndex]->GetVelocity();
            v = XMVectorSubtract(centre, boidVelocity) * m_boidMatchingFactor;
        }

        return v;
    }
}

DirectX::XMVECTOR Swarm::ExecuteRule4(int boidIndex)
{
    XMFLOAT3 v;
    ZeroMemory(&v, sizeof(v));
    XMFLOAT3 pos;
    ZeroMemory(&pos, sizeof(pos));
    XMStoreFloat3(&pos, m_boids[boidIndex]->GetPosition());

    // Keeps the boid within bounds. The boids can fly out of boundaries, but then slowly turn back, avoiding any harsh motions
    if (pos.x < BOUNDARY_X_MIN)
        v.x = BOID_TURN_FACTOR;
    else if (pos.x > BOUNDARY_X_MAX)
        v.x = -BOID_TURN_FACTOR;

    if (pos.y < BOUNDARY_Y_MIN)
        v.y = BOID_TURN_FACTOR;
    else if (pos.y > BOUNDARY_Y_MAX)
        v.y = -BOID_TURN_FACTOR;

    if (pos.z < BOUNDARY_Z_MIN)
        v.z = BOID_TURN_FACTOR;
    else if (pos.z > BOUNDARY_Z_MAX)
        v.z = -BOID_TURN_FACTOR;

    return XMLoadFloat3(&v);
}

std::tuple<DirectX::XMVECTOR, DirectX::XMVECTOR> Swarm::GetRandomPositionAndVelocity()
{
    XMVECTOR randomPosition = XMVectorSet(
        m_rand->GetFloat(8 * BOUNDARY_X_MIN, 8 * BOUNDARY_X_MAX),
        m_rand->GetFloat(2.1f * BOUNDARY_Y_MAX, 7 * BOUNDARY_Y_MAX),
        m_rand->GetFloat(2.5f * BOUNDARY_Z_MIN, 10 * BOUNDARY_Z_MIN),
        0);

    XMVECTOR randomVelocity = m_maxBoidSpeed *
        XMVector4Normalize(
            XMVectorSet(
                m_rand->GetFloat(-1, 1),
                m_rand->GetFloat(-1, 1),
                m_rand->GetFloat(-1, 1),
                0));

    return { randomPosition, randomVelocity };
}
    