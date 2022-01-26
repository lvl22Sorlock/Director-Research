# Director-Research

##Intro
The aim of this project is to create a director AI for an agar.io inspired game.
There is 1 skilled 'Uber Agent' that actively tries to win,
while the other agents just wander around aimlessly.

The director AI makes the game easier
for the losing party so, even though the 'Uber Agent'
is far more skilled than the others, their chance of winning is about the same.

##Implementation
To do this I gave the agents a new member variable : 
![image](https://user-images.githubusercontent.com/96822243/151254252-932a0e84-de71-4ff1-9cfd-9da550bdda1d.png)
When the agent eats food or a smaller agent, the size increase gets multiplied by this value.


Every frame I compare the 'Uber Agent' to the largest opponent,
then I use the size of the uber agent divided by the size of the largest opponent
to calculate a maximum multiplier for all agents :
![image](https://user-images.githubusercontent.com/96822243/151254762-a1f4b361-e2ec-4f58-84be-81e789eab5a9.png)

I then use this to calculate the modifier for all enemies and clamp it
so it isn't larget than this maximum.
This way even really small opponents can't get an incredibly high multiplier,
because of this it will always be a battle between the 'Uber Agent' and 1 powerful opponent.
If all agents were about the same size because could get really high multipliers 
then it would seem a bit unrealistic and the use of a multiplier would be too obvious.

The uber agent always has a multiplier of 1.
![image](https://user-images.githubusercontent.com/96822243/151255444-c0414d15-081f-4c1d-81e1-7e664b562881.png)

##Result
The result is that, even though the 'Uber Agent' if far better in the game,
the opponents will still be able to keep up.
And even though all opponents gain a bonus, the opponents sizes are still rather varied
so it's not immediately obvious that they have an advantage (or disadvantage)
based on 'Uber Agent' 's succes.

![ResearchMatch](https://user-images.githubusercontent.com/96822243/151256119-b30e6666-3223-45ac-88ba-adf491e887dc.png)

