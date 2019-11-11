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

#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include <vector>

enum class EConvexAlg
{
	JARVIS,
	GRAHAM,
	DAC
};

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
	Node* CreatePoint(cocos2d::Vec2 position = cocos2d::Vec2::ZERO);
	void Setup(const uint32_t numPoints);

	void OnKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void OnMouseMove(cocos2d::Event* event);
	void OnMouseDown(cocos2d::Event* event);
	void OnMouseUp(cocos2d::Event* event);

	void update(float dt) override;
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

private:

	std::vector<cocos2d::Vec2> m_Set;
	std::vector<cocos2d::Vec2> m_Hull;
	std::vector<cocos2d::Vec2> m_MoveDirections;
	std::vector<long long> m_JarvisTimes;
	std::vector<long long> m_GrahamTimes;
	std::vector<long long> m_DACTimes;

	cocos2d::Size m_FieldSize;
	cocos2d::Vec2 m_DistPoint;
	cocos2d::DrawNode* m_DrawNode;
	cocos2d::Label* m_TimeLabel;

	uint32_t m_NumToSetup{ 0 };

	EConvexAlg m_CurrentAlg{ EConvexAlg::JARVIS };

	bool m_NeedSetup{ false };
	bool m_MouseDown{ false };
	bool m_RightMousDown{ false };
};

#endif // __HELLOWORLD_SCENE_H__
