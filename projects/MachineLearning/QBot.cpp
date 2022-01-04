#include "stdafx.h"
#include "QBot.h"
#include "Food.h"

// for setting the precision in cout for floating points.
#include <iomanip>

QBot::QBot(float x,
	float y,
	float fov,
	float sFov,
	float angle,
	int memorySize,
	int nrInputs,
	int nrOutputs,
	bool useBias) :
	m_Location(x, y), m_StartLocation(x, y), m_FOV(fov), m_SFOV(sFov), m_Angle(angle),
	m_AliveColor(0.1f, 0.5f, .5f),
	m_DeadColor(.75f, 0.1f, .2f),
	m_NrOfInputs(nrInputs),
	m_NrOfOutputs(nrOutputs),
	m_MemorySize(memorySize),
	m_UseBias(useBias),
	m_BotBrain(nrInputs + (useBias ? 1 : 0), nrOutputs),
	m_DeltaBotBrain(nrInputs + (useBias ? 1 : 0), nrOutputs),
	m_SAngle(1, nrOutputs)
{
	float start = -m_SFOV / 2;
	float step = m_SFOV / (nrOutputs - 1);
	for (int i = 0; i < nrOutputs; ++i)
	{
		float value = start + i * step;
		m_SAngle.Set(0, i, value);
	}

	m_ActionMatrixMemoryArr = new Elite::FMatrix[m_MemorySize];
	m_StateMatrixMemoryArr = new Elite::FMatrix[m_MemorySize];

	for (int i = 0; i < m_MemorySize; ++i)
	{
		m_StateMatrixMemoryArr[i].Resize(1, m_NrOfInputs + (m_UseBias ? 1 : 0));
		m_ActionMatrixMemoryArr[i].Resize(1, m_NrOfOutputs);
	}
	m_BotBrain.Randomize(-1.0f, 2.0f);
	if (m_UseBias) {
		m_BotBrain.SetRowAll(m_NrOfInputs, -10.0f);
	}

	m_BotBrain.Print();
}

QBot::~QBot() 
{
	delete[] m_ActionMatrixMemoryArr;
	delete[] m_StateMatrixMemoryArr;
}

void QBot::Update(vector<Food*>& foodList, float deltaTime)
{
	m_Age += deltaTime;
	currentIndex = (currentIndex + 1) % m_MemorySize;
	if (!m_Alive) {
		return;
	}
	m_Visible.clear();
	Elite::Vector2 dir(cos(m_Angle), sin(m_Angle));
	float angleStep = m_FOV / (m_NrOfInputs);
	m_StateMatrixMemoryArr[currentIndex].SetAll(0.0);
	bool cameClose = false;
	for (Food* food : foodList) {
		if (food->IsEaten()) {
			continue;
		}
		Vector2 foodLoc = food->GetLocation();
		Vector2 foodVector = foodLoc - (m_Location - dir * 10);
		float dist = (foodLoc-m_Location).Magnitude();
		if (dist > m_MaxDistance) {
			continue;
		}
		foodVector *= 1 / dist;

		float angle = AngleBetween(dir, foodVector);
		if (angle > -m_FOV / 2 && angle < m_FOV / 2) {
			m_Visible.push_back(food);

			int index = (int)((angle + m_FOV / 2) / angleStep);
			float invDist = CalculateInverseDistance(dist);
			float currentDist = m_StateMatrixMemoryArr[currentIndex].Get(0, index);
			if (invDist > currentDist) {
				m_StateMatrixMemoryArr[currentIndex].Set(0, index, invDist);
			}
		}
		else if (dist < 10.0f) {
			cameClose = true;
		}
		if (dist < 2.0f) {
			food->Eat();
			m_CameCloseCounter = 50;
			m_FoodEaten++;
			m_Health += 30.0f;
			Reinforcement(m_PositiveQ,m_MemorySize);
		}
	}

	if (m_CameCloseCounter > 0) {
		m_CameCloseCounter--;
	}

	if (cameClose && m_CameCloseCounter == 0) {
		Reinforcement(m_NegativeQClose,m_MemorySize);
		m_CameCloseCounter = 50;
	}


	m_StateMatrixMemoryArr[currentIndex].Set(0, m_NrOfInputs, 1); //bias
	m_StateMatrixMemoryArr[currentIndex].MatrixMultiply(m_BotBrain, m_ActionMatrixMemoryArr[currentIndex]);
	m_ActionMatrixMemoryArr[currentIndex].Sigmoid();

	int r, c;
	float max = m_ActionMatrixMemoryArr[currentIndex].Max(r, c);

	float dAngle = m_SAngle.Get(0, c);
	m_Angle += dAngle *deltaTime;

	Elite::Vector2 newDir(cos(m_Angle), sin(m_Angle));
	m_Location += newDir * m_Speed*deltaTime;

	m_Health -= 0.1f;
	if (m_Health < 0) {
		// update the bot brain, something went wrong.
		Reinforcement(m_NegativeQ,m_MemorySize);
		m_Health = 100.0f;
		m_Location = m_StartLocation;
		
		cout << "Died after "<< std::setprecision(4) << m_Age << " seconds." << endl;
		m_Age = 0;
	}
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(m_Location);
}

void QBot::Render(float deltaTime) {
	Elite::Vector2 dir(cos(m_Angle), sin(m_Angle));
	Elite::Vector2 leftVision(cos(m_Angle + m_FOV / 2), sin(m_Angle + m_FOV / 2));
	Elite::Vector2 rightVision(cos(m_Angle - m_FOV / 2), sin(m_Angle - m_FOV / 2));

	Elite::Vector2 perpDir(-dir.y, dir.x);

	Color c = m_DeadColor;
	if (m_Alive) {
		c = m_AliveColor;
	}

	DEBUGRENDERER2D->DrawSolidCircle(m_Location, 2, dir, c);
	if (m_Alive) {
		DEBUGRENDERER2D->DrawSegment(m_Location - 10 * dir, m_Location + m_MaxDistance * leftVision, c);
		DEBUGRENDERER2D->DrawSegment(m_Location - 10 * dir, m_Location + m_MaxDistance * rightVision, c);
	}
	DEBUGRENDERER2D->DrawString(m_Location, to_string((int)m_Health).c_str());

	if (m_Alive) {
		for (Food* f : m_Visible) {
			Vector2 loc = f->GetLocation();
			DEBUGRENDERER2D->DrawCircle(loc, 2, c, 0.5f);
		}
	}

	// draw the vision
	for (int i = 0; i < m_NrOfInputs; ++i)
	{

		if (m_StateMatrixMemoryArr[currentIndex].Get(0, i) > 0.0f) {
			DEBUGRENDERER2D->DrawSolidCircle(m_Location - 2.5 * dir - perpDir * 2.0f * (i - m_NrOfInputs / 2.0f), 1, perpDir, m_AliveColor);
		}
		else {
			DEBUGRENDERER2D->DrawSolidCircle(m_Location - 3.0 * dir - perpDir * 2.0f * (i - m_NrOfInputs / 2.0f), 1, perpDir, m_DeadColor);
		}
	}

	char age[10];
	snprintf(age, 10, "%.1f seconds", m_Age);
	DEBUGRENDERER2D->DrawString(m_Location + m_MaxDistance * dir, age);
}

bool QBot::IsAlive()
{
	return m_Alive;
}

void QBot::Reset() 
{
	m_Health = 100;
	m_TimeOfDeath = 0;
	m_Alive = true;
	m_FoodEaten = 0;

	float startx = Elite::randomFloat(-50.0f, 50.0f);
	float starty = Elite::randomFloat(-50.0f, 50.0f);
	float startAngle = Elite::randomFloat(0, float(M_PI) * 2);
}

float QBot::CalculateFitness() const 
{
	return m_FoodEaten + m_TimeOfDeath;
}

//void QBot::MutateMatrix(Generation* gen, Elite::FMatrix& matrix, float mutationRate, float mutationAmplitude) 
//{
//	for (int c = 0; c < matrix.GetNrOfColumns(); ++c) 
//	{
//		for (int r = 0; r < matrix.GetNrOfRows(); ++r) 
//		{
//			if (gen->Random(0, 1) < mutationRate) 
//			{
//				float update = gen->Random(-mutationAmplitude, mutationAmplitude);
//				float currentVal = matrix.Get(r, c);
//				matrix.Set(r, c, currentVal + update);
//			}
//		}
//	}
//}

void QBot::Reinforcement(float factor,int memory)
{
	// go back in time, and reinforce (or inhibit) the weights that led to the right/wrong decision.
	#pragma region Step 1
	m_DeltaBotBrain.SetAll(0);
	#pragma endregion
	
	#pragma region Step 2
	// Calculate min of m_MemorySize and memoryUpdate
	int minMemory{min(m_MemorySize, memory)};
	#pragma endregion

	#pragma region Step 3
	for (int mi = 0; mi < minMemory; ++mi)
	{
		float timeFactor{
			1.0f /
				(1 
				+
				(mi*mi))};


		int actualIndex{mi};
		float cMax{ m_ActionMatrixMemoryArr->MaxOfRow(actualIndex) };

		for (int c = 0; c < m_StateMatrixMemoryArr->GetNrOfColumns(); ++c)
		{
			float scVal{ m_StateMatrixMemoryArr->Get(0, c) };

			if (scVal > 0)
			{
				m_DeltaBotBrain.Add(c, cMax, timeFactor * factor * scVal);

				int i{ 0 };
				int rcMax{};
				do
				{
					++i;
					rcMax = rand() % m_ActionMatrixMemoryArr->GetNrOfColumns();
					assert(i < 100);
				} while (m_ActionMatrixMemoryArr->Get(actualIndex, rcMax) >= cMax);
				if (rcMax < 0) std::cout << "mistake here\n";
				assert(rcMax >= 0);

				m_DeltaBotBrain.Add(c, rcMax, -timeFactor * factor * scVal);
			}
		}
	}
	#pragma endregion

	#pragma region Step 4
	m_DeltaBotBrain.ScalarMultiply(1 / m_MemorySize);
	#pragma endregion

	#pragma region Step 5
	m_BotBrain.Add(m_DeltaBotBrain);
	#pragma endregion

}


float QBot::CalculateInverseDistance(float realDist) 
{
	// version 1 
	//return m_MaxDistance - realDist;
	// version 2
	float nDist = realDist / m_MaxDistance;
	float invDistSquared = m_MaxDistance / (1 + nDist * nDist);
	return invDistSquared;
}