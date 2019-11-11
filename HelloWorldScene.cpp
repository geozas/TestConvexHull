/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <thread>
#include <chrono>
#include "ConvexHullAlgs.hpp"

USING_NS_CC;

static std::random_device s_RD;
static std::mt19937 s_MTE(s_RD());
std::uniform_real_distribution<float> dirDist(-1.0f, 1.0f);
std::uniform_real_distribution<float> spdDist(-500.0f, 500.0f);

std::vector<long long> m_JarvisTimes;
std::vector<long long> m_GrahamTimes;
std::vector<long long> m_DACTimes;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}


Node* HelloWorld::CreatePoint(Vec2 position)
{
	auto point = Node::create();
	point->setPosition(position);
	addChild(point);

	auto physics = PhysicsBody::createBox(Size{ 1.0f, 1.0f }, PhysicsMaterial(0.0f, 1.0f, 0.0f));
	physics->setDynamic(true);
	physics->setCollisionBitmask(1);
	physics->setGravityEnable(false);
	physics->setLinearDamping(0.0f);

	point->setPhysicsBody(physics);

	return point;
}

void HelloWorld::Setup(const uint32_t numPoints)
{
	m_JarvisTimes.clear();
	m_GrahamTimes.clear();
	m_DACTimes.clear();

	m_Set.clear();
	m_MoveDirections.clear();

	m_Set.reserve(numPoints);
	m_MoveDirections.reserve(numPoints);
	m_Hull.reserve(numPoints);

	std::uniform_real_distribution<float> positionXDist(400.0f, m_FieldSize.width - 400.0f);
	std::uniform_real_distribution<float> positionYDist(200.0f, m_FieldSize.height - 200.0f);

	std::uniform_real_distribution<float> dirDist(-1.0f, 1.0f);

	for (uint32_t i = 0; i < numPoints; ++i)
	{
		m_Set.emplace_back(positionXDist(s_MTE), positionYDist(s_MTE));

		Vec2 dir{ dirDist(s_MTE), dirDist(s_MTE) };
		dir.normalize();
		m_MoveDirections.push_back(dir);
	}

}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

	m_JarvisTimes.reserve(999999);
	m_GrahamTimes.reserve(999999);
	m_DACTimes.reserve(999999);

    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();

	m_FieldSize = visibleSize;

	m_DrawNode = DrawNode::create();
	addChild(m_DrawNode);

	m_TimeLabel = Label::createWithSystemFont("My", "Arial", 30);
	addChild(m_TimeLabel, 100);
	m_TimeLabel->setPosition(400.0f, 400.0f);

	auto listener = EventListenerKeyboard::create();
	listener->onKeyPressed = CC_CALLBACK_2(HelloWorld::OnKeyPressed, this);

	auto _mouseListener = EventListenerMouse::create();
	_mouseListener->onMouseMove = CC_CALLBACK_1(HelloWorld::OnMouseMove, this);
	_mouseListener->onMouseUp = CC_CALLBACK_1(HelloWorld::OnMouseUp, this);
	_mouseListener->onMouseDown = CC_CALLBACK_1(HelloWorld::OnMouseDown, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener, this);


	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	scheduleUpdate();

    return true;
}

void HelloWorld::update(float dt)
{
	printf("");
	m_DrawNode->clear();

	if (!m_Set.empty())
	{
		m_DrawNode->drawLine(m_Set[0], m_Set[0] + Vec2{ 1.0f, 1.0f }, Color4F::RED);
	}


	for (uint32_t i = 1; i < m_Set.size(); ++i)
	{
		const float spd = spdDist(s_MTE);

		Vec2& point = m_Set[i];

		if (m_RightMousDown)
		{
			auto dir = point - m_DistPoint;
			float lenght = dir.getLength();
			dir.normalize();
			point += dir * ((30000.0f / lenght) * dt);
		}

		const std::size_t eachN = m_Set.size() * 0.1f;

		if (i % eachN == 0)
		{
			Vec2& dir = m_MoveDirections[i];
			point += dir * (dt * 100.0f);

			if (point.x > m_FieldSize.width - 50.0f || point.x < 50.0f)
			{
				dir.x *= -1.0f;
			}

			if (point.y > m_FieldSize.height - 50.0f || point.y < 50.0f)
			{
				dir.y *= -1.0f;
			}

			m_DrawNode->drawLine(point, point + Vec2{ 1.0f, 1.0f }, Color4F::YELLOW);
		}
		else
		{
			point.x += dirDist(s_MTE) * (dt * spd);
			point.y += dirDist(s_MTE) * (dt * spd);

			m_DrawNode->drawLine(point, point + Vec2{ 1.0f, 1.0f }, Color4F::WHITE);
		}

		const float fieldLeftBorder = 10.0f;
		const float fieldRightBorder = m_FieldSize.width - 10.0f;

		if (point.x < fieldLeftBorder)
		{
			point.x = fieldLeftBorder + 10.0f;
		}
		else if (point.x > fieldRightBorder)
		{
			point.x = fieldRightBorder - 10.0f;
		}

		const float fieldUpBorder = 10.0f;
		const float fieldBottomBorder = m_FieldSize.height - 10.0f;


		if (point.y < fieldUpBorder)
		{
			point.y = fieldUpBorder + 10.0f;
		}
		else if (point.y > fieldBottomBorder)
		{
			point.y = fieldBottomBorder - 10.0f;
		}

	}
	
	m_Hull.clear();

	Color4F algColor = Color4F::GREEN;

	auto start = std::chrono::high_resolution_clock::now();
	decltype(start) end;
	decltype(end - start) diff;

	long long avg = 0;

	switch (m_CurrentAlg)
	{
	case EConvexAlg::JARVIS:
		GetConvexHull_Jarvis(m_Set, m_Hull);
		algColor = Color4F::GREEN;
		end = std::chrono::high_resolution_clock::now();
		diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		m_JarvisTimes.push_back(diff.count());
		avg = std::accumulate(m_JarvisTimes.cbegin(), m_JarvisTimes.cend(), 0ll) / m_JarvisTimes.size();
		break;
	case EConvexAlg::GRAHAM:
		GetConvexHull_Graham(m_Set, m_Hull);
		algColor = Color4F::RED;
		end = std::chrono::high_resolution_clock::now();
		diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		m_GrahamTimes.push_back(diff.count());
		avg = std::accumulate(m_GrahamTimes.cbegin(), m_GrahamTimes.cend(), 0ll) / m_GrahamTimes.size();
		break;
	case EConvexAlg::DAC:
		GetConvexHull_DAC(m_Set, m_Hull);
		algColor = Color4F::BLUE;
		end = std::chrono::high_resolution_clock::now();
		diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		m_DACTimes.push_back(diff.count());
		avg = std::accumulate(m_DACTimes.cbegin(), m_DACTimes.cend(), 0ll) / m_DACTimes.size();
		break;
	}


	m_TimeLabel->setString(std::to_string(avg));

	

	for (uint32_t i = 0; i < m_Hull.size(); ++i)
	{
		const Vec2 nextPoint = ((i + 1) == m_Hull.size()) ? m_Hull[0] : m_Hull[i + 1];

		m_DrawNode->drawSegment(m_Hull[i], nextPoint, 5.0f, algColor);
	}
}

void HelloWorld::OnKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_Q:
		Setup(1u);
		break;
	case EventKeyboard::KeyCode::KEY_W:
		Setup(10u);
		break;
	case EventKeyboard::KeyCode::KEY_E:
		Setup(100u);
		break;
	case EventKeyboard::KeyCode::KEY_R:
		Setup(1000u);
		break;
	case EventKeyboard::KeyCode::KEY_T:
		Setup(10000u);
		break;
	case EventKeyboard::KeyCode::KEY_Y:
		Setup(100000u);
		break;
	case EventKeyboard::KeyCode::KEY_U:
		Setup(1000000u);
		break;
	case EventKeyboard::KeyCode::KEY_I:
		Setup(10000000u);
		break;
	case EventKeyboard::KeyCode::KEY_O:
		Setup(100000000u);
		break;
	case EventKeyboard::KeyCode::KEY_P:
		Setup(1000000000u);
		break;
	case EventKeyboard::KeyCode::KEY_A:
		m_CurrentAlg = EConvexAlg::JARVIS;
		break;
	case EventKeyboard::KeyCode::KEY_S:
		m_CurrentAlg = EConvexAlg::GRAHAM;
		break;
	case EventKeyboard::KeyCode::KEY_D:
		m_CurrentAlg = EConvexAlg::DAC;
		break;
	case EventKeyboard::KeyCode::KEY_N:
		pauseSchedulerAndActions();
		break;
	case EventKeyboard::KeyCode::KEY_B:
		resumeSchedulerAndActions();
		break;
	default:
		break;
	}
}

void HelloWorld::OnMouseMove(cocos2d::Event* event)
{
	EventMouse* e = (EventMouse*)event;
	if (m_Set.size() > 0 && m_MouseDown)
	{
		m_Set[0] = { e->getCursorX(), e->getCursorY() + 1080.0f };
	}
}

void HelloWorld::OnMouseDown(cocos2d::Event* event)
{
	EventMouse* e = (EventMouse*)event;

	if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
	{
		m_MouseDown = true;

		if (m_Set.size() > 0)
		{
			m_Set[0] = { e->getCursorX(), e->getCursorY() + 1080.0f };
		}
	}
	else
	{
		m_RightMousDown = true;
		m_DistPoint = { e->getCursorX(), e->getCursorY() + 1080.0f };
	}

}

void HelloWorld::OnMouseUp(cocos2d::Event* event)
{
	m_MouseDown = false;
	m_RightMousDown = false;
}
