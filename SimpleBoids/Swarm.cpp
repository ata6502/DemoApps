#include "pch.h"

#include "Constants.h"
#include "Swarm.h"

using namespace Concurrency;
using namespace DirectX;

Swarm::Swarm(
    float boidRadius, 
    float boidMinDistance, 
    float boidMatchingFactor, 
    float maxBoidSpeed,
    float boidAvoidFactor,
    float boidTurnFactor,
    float boidVisualRange,
    float boidMoveToCenterFactor,
    float boxEdgeLength) :
    m_boidRadius(boidRadius),
    m_isVisualRangeEnabled(false),
    m_boxEdgeLength(boxEdgeLength)
{
    m_boidParameters[BoidParameter::MinDistance] = boidMinDistance;
    m_boidParameters[BoidParameter::MatchingFactor] = boidMatchingFactor;
    m_boidParameters[BoidParameter::MaxSpeed] = maxBoidSpeed;
    m_boidParameters[BoidParameter::AvoidFactor] = boidAvoidFactor;
    m_boidParameters[BoidParameter::TurnFactor] = boidTurnFactor;
    m_boidParameters[BoidParameter::VisualRange] = boidVisualRange;
    m_boidParameters[BoidParameter::MoveToCenterFactor] = boidMoveToCenterFactor;

    m_rand = std::make_unique<RandomNumberHelper>();
}

void Swarm::AddBoids(int count)
{
    critical_section::scoped_lock lock(m_criticalSection);

    for (auto i = 0; i < count; ++i)
    {
        auto [randomPosition, randomVelocity] = GetRandomPositionAndVelocity();
        m_boids.emplace_back(std::make_unique<Boid>(randomPosition, randomVelocity, GetBoidParameter(BoidParameter::MaxSpeed)));
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

    XMVECTOR v1, v2, v3, v4;

    for (int i = 0; i < Size(); ++i)
    {
        // Perform vector operations on the positions of the boids. Operations are independent from each other.

        // Rule 1: Make boids fly towards the centre of the mass of neighbouring boids.
        v1 = ExecuteRule1(i);

        // Rule 2: Move away from other boids that are too close to avoid colliding.
        v2 = ExecuteRule2(i);

        // Rule 3: Find the average velocity (speed and direction) of the other boids and adjust velocity slightly to match.
        v3 = ExecuteRule3(i);

        // Rule 4: Encourage boids to stay within rough boundaries.
        v4 = ExecuteRule4(i);

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

void Swarm::SetBoidParameter(BoidParameter parameter, float value)
{
    m_boidParameters[parameter] = value;

    if (parameter == BoidParameter::MaxSpeed)
        SetMaxBoidSpeed(value);
}

// Move the boid toward its 'perceived centre', which is the centre of all the other boids, not including itself.
DirectX::XMVECTOR Swarm::ExecuteRule1(int boidIndex)
{
    // Add all boids' positions except the boid with the boidIndex.
    XMVECTOR sum{ XMVectorZero() };
    for (int i = 0; i < Size(); ++i)
    {
        if (boidIndex != i)
            sum = XMVectorAdd(sum, m_boids[i]->GetPosition());
    }

    size_t boidCount = Size() - 1; // all the boids minus the current boid
    XMVECTOR centre = sum / (static_cast<float>(boidCount)); // the center of mass
    XMVECTOR boidPosition = m_boids[boidIndex]->GetPosition();
    XMVECTOR v = XMVectorSubtract(centre, boidPosition) * GetBoidParameter(BoidParameter::MoveToCenterFactor);

    return v;
}

// Move away from other boids that are too close to avoid colliding.
DirectX::XMVECTOR Swarm::ExecuteRule2(int boidIndex)
{
    XMVECTOR moveDelta{ XMVectorZero() };
    float minDistance = m_boidRadius + GetBoidParameter(BoidParameter::MinDistance);

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
            if (distance < minDistance)
                moveDelta = XMVectorSubtract(moveDelta, diff);
        }
    }

    XMVECTOR v = GetBoidParameter(BoidParameter::AvoidFactor) * moveDelta;
    return v;
}

// Adjust the boid's velocity to match the average velocity of the other boids.
DirectX::XMVECTOR Swarm::ExecuteRule3(int boidIndex)
{
    float matchingFactor = GetBoidParameter(BoidParameter::MatchingFactor);

    // Take into account only boids in a certain range from a given boid.
    if (m_isVisualRangeEnabled)
    {
        XMVECTOR avg{ XMVectorZero() };
        int neighborCount = 0;
        float visualRange = GetBoidParameter(BoidParameter::VisualRange);

        for (int i = 0; i < Size(); ++i)
        {
            if (boidIndex != i)
            {
                // Calculate the distance of this boid to the other boid.
                auto p1 = m_boids[boidIndex]->GetPosition();
                auto p2 = m_boids[i]->GetPosition();
                auto diff = XMVectorSubtract(p2, p1);
                auto distance = XMVectorGetX(XMVector3Length(diff));

                if (distance < visualRange)
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
            v = XMVectorSubtract(centre, boidVelocity) * matchingFactor;
        }

        return v;


    }
    // Take into account all boids.
    else
    {
        XMVECTOR sum{ XMVectorZero() };

        for (int i = 0; i < Size(); ++i)
        {
            if (boidIndex != i)
                sum = XMVectorAdd(sum, m_boids[i]->GetVelocity());
        }

        size_t boidCount = Size() - 1; // all the boids minus the current boid
        XMVECTOR centre = sum / static_cast<float>(boidCount);
        XMVECTOR boidVelocity = m_boids[boidIndex]->GetVelocity();
        XMVECTOR v = XMVectorSubtract(centre, boidVelocity) * matchingFactor;

        return v;
    }
}

// Keeps the boid within bounds. The boids can fly out of boundaries, but then slowly turn back, avoiding any harsh motions
DirectX::XMVECTOR Swarm::ExecuteRule4(int boidIndex)
{
    XMFLOAT3 v;
    ZeroMemory(&v, sizeof(v));
    XMFLOAT3 pos;
    ZeroMemory(&pos, sizeof(pos));
    XMStoreFloat3(&pos, m_boids[boidIndex]->GetPosition());

    float turnFactor = GetBoidParameter(BoidParameter::TurnFactor);

    if (pos.x < -m_boxEdgeLength)
        v.x = turnFactor;
    else if (pos.x > m_boxEdgeLength)
        v.x = -turnFactor;

    if (pos.y < -m_boxEdgeLength)
        v.y = turnFactor;
    else if (pos.y > m_boxEdgeLength)
        v.y = -turnFactor;

    if (pos.z < -m_boxEdgeLength)
        v.z = turnFactor;
    else if (pos.z > m_boxEdgeLength)
        v.z = -turnFactor;

    return XMLoadFloat3(&v);
}

std::tuple<DirectX::XMVECTOR, DirectX::XMVECTOR> Swarm::GetRandomPositionAndVelocity()
{
    XMVECTOR randomPosition = XMVectorSet(
        m_rand->GetFloat(-8.0f * m_boxEdgeLength, 8.0f * m_boxEdgeLength),
        m_rand->GetFloat(2.0f * m_boxEdgeLength, 5.0f * m_boxEdgeLength),
        m_rand->GetFloat(-2.0f * m_boxEdgeLength, -6.5f * m_boxEdgeLength),
        0);

    XMVECTOR randomVelocity = GetBoidParameter(BoidParameter::MaxSpeed) *
        XMVector4Normalize(
            XMVectorSet(
                m_rand->GetFloat(-1, 1),
                m_rand->GetFloat(-1, 1),
                m_rand->GetFloat(-1, 1),
                0));

    return { randomPosition, randomVelocity };
}

void Swarm::SetMaxBoidSpeed(float maxBoidSpeed)
{
    // Update max speed of all boids in the swarm.
    for (auto i = 0; i < Size(); ++i)
    {
        m_boids[i]->SetMaxSpeed(maxBoidSpeed);
    }
}
    