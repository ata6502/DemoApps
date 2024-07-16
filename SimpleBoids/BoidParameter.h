#pragma once

enum class BoidParameter
{
    MinDistance,        // the minimum distance between boids
    MaxSpeed,           // the max length of the velocity vector
    MoveToCenterFactor, // determines how to move a boid towards the center (percentage)
    AvoidFactor,        // scales the vectors that repel boids
    TurnFactor,         // encourages boids to fly in a particular direction
    VisualRange,        // used in calculating boid's velocity while taking into account only boids in a certain range
    MatchingFactor,     // adjustment of average velocity as % (boid matching factor)
};
