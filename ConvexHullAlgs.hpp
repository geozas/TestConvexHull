#pragma once

#include "math/Vec2.h"
#include <vector>
#include <random>
#include "cocos2d.h"


bool IsCCW(cocos2d::Vec2 const a, cocos2d::Vec2 const b, cocos2d::Vec2 const c)
{
	const cocos2d::Vec2 vec1 = { a.x - b.x, a.y - b.y };
	const cocos2d::Vec2 vec2 = { c.x - b.x, c.y - b.y };
	return vec1.cross(vec2) < 0;
}

void GetConvexHull_Jarvis(std::vector<cocos2d::Vec2> const& set, std::vector<cocos2d::Vec2>& hull)
{
	if (set.size() < 3)
	{
		std::copy(set.cbegin(), set.cend(), std::back_inserter(hull));
		return;
	}

	cocos2d::Vec2 hullPoint = *std::min_element(set.cbegin(), set.cend(), [](cocos2d::Vec2 const lhs, cocos2d::Vec2 const rhs) noexcept { return lhs.x < rhs.x; });

	cocos2d::Vec2 endPoint;

	do
	{
		hull.push_back(hullPoint);
		endPoint = set.front();

		for (uint32_t i = 1; i < set.size(); ++i)
		{
			const cocos2d::Vec2 testedPoint = set[i];
			if (IsCCW(endPoint, hullPoint, testedPoint) || hullPoint == endPoint)
			{
				endPoint = testedPoint;
			}
		}
		hullPoint = endPoint;

	} while (endPoint != hull.front());
}

void GetConvexHull_Graham_Inplace(std::vector<cocos2d::Vec2>& set, std::vector<cocos2d::Vec2>& hull)
{
	if (set.size() < 2)
	{
		return;
	}

	const cocos2d::Vec2 leftmost = *std::min_element(set.cbegin(), set.cend(), [](cocos2d::Vec2 const lhs, cocos2d::Vec2 const rhs) noexcept { return lhs.x < rhs.x; });

	std::sort(set.begin(), set.end(), [leftmost](cocos2d::Vec2 const lhs, cocos2d::Vec2 const rhs) { return IsCCW(rhs, leftmost, lhs) || lhs == leftmost; });

	for (auto point : set)
	{
		while (hull.size() > 1 && IsCCW(point, *hull.rbegin(), *(hull.rbegin() + 1)))
		{
			hull.pop_back();
		}
		hull.push_back(point);
	}
}

void GetConvexHull_Graham(const std::vector<cocos2d::Vec2>& set, std::vector<cocos2d::Vec2>& hull)
{
	if (set.size() <= 3)
	{
		return;
	}

	std::vector<cocos2d::Vec2> initialSet(set);

	const cocos2d::Vec2 leftmost = *std::min_element(initialSet.cbegin(), initialSet.cend(), [](cocos2d::Vec2 const lhs, cocos2d::Vec2 const rhs) noexcept { return lhs.x < rhs.x; });

	std::sort(initialSet.begin(), initialSet.end(), [leftmost](cocos2d::Vec2 const lhs, cocos2d::Vec2 const rhs) { return IsCCW(rhs, leftmost, lhs) || lhs == leftmost; });

	for (auto point : initialSet)
	{
		while (hull.size() > 1 && IsCCW(point, *hull.rbegin(), *(hull.rbegin() + 1)))
		{
			hull.pop_back();
		}
		hull.push_back(point);
	}
}

auto CCWNext(std::vector<cocos2d::Vec2> const& hull, std::vector<cocos2d::Vec2>::const_iterator it) { return ((it + 1) != hull.end()) ? (it + 1) : hull.begin(); }
auto CWNext(std::vector<cocos2d::Vec2> const& hull, std::vector<cocos2d::Vec2>::const_iterator it) { return (it != hull.begin()) ? (it - 1) : (hull.end() - 1); }



void Merge_DAC(std::vector<cocos2d::Vec2> const& lhs, std::vector<cocos2d::Vec2> const& rhs, std::vector<cocos2d::Vec2>& hull, cocos2d::DrawNode* deb)
{
	if (lhs.empty())
	{
		std::copy(rhs.cbegin(), rhs.cend(), std::back_inserter(hull));
		return;
	}
	if (rhs.empty())
	{
		std::copy(lhs.cbegin(), lhs.cend(), std::back_inserter(hull));
		return;
	}

	auto rightmostLeft = std::max_element(lhs.cbegin(), lhs.cend(), [](cocos2d::Vec2 const lhs, cocos2d::Vec2 const rhs) { return lhs.x < rhs.x; });
	auto leftmostRight = std::min_element(rhs.cbegin(), rhs.cend(), [](cocos2d::Vec2 const lhs, cocos2d::Vec2 const rhs) { return lhs.x < rhs.x; });

	auto upperTangentLeft = rightmostLeft;
	auto upperTangentRight = leftmostRight;

	auto lowerTangentLeft = rightmostLeft;
	auto lowerTangentRight = leftmostRight;

	bool luDone = false;
	bool ruDone = false;

	while (!luDone || !ruDone)
	{
		auto nextRight = CWNext(rhs, upperTangentRight);
		if (rhs.size() != 1 && IsCCW(*upperTangentLeft, *upperTangentRight, *nextRight))
		{
			upperTangentRight = nextRight;
			ruDone = false;
		}
		else
		{
			ruDone = true;
		}

		auto nextLeft = CCWNext(lhs, upperTangentLeft);
		if (lhs.size() != 1 && IsCCW(*nextLeft, *upperTangentLeft, *upperTangentRight))
		{
			upperTangentLeft = nextLeft;
			luDone = false;
		}
		else
		{
			luDone = true;
		}
	}

	bool llDone = false;
	bool rlDone = false;

	while (!llDone || !rlDone)
	{
		auto nextRight = CCWNext(rhs, lowerTangentRight);
		if (rhs.size() != 1 && !IsCCW(*lowerTangentLeft, *lowerTangentRight, *nextRight))
		{
			lowerTangentRight = nextRight;
			rlDone = false;
		}
		else
		{
			rlDone = true;
		}

		auto nextLeft = CWNext(lhs, lowerTangentLeft);
		if (lhs.size() != 1 && !IsCCW(*nextLeft, *lowerTangentLeft, *lowerTangentRight))
		{
			lowerTangentLeft = nextLeft;
			llDone = false;
		}
		else
		{
			llDone = true;
		}
	}

	auto it1 = upperTangentLeft;
	hull.push_back(*upperTangentLeft);
	while (it1 != lowerTangentLeft)
	{
		it1 = CCWNext(lhs, it1);
		hull.push_back(*it1);
	}

	auto it2 = lowerTangentRight;
	hull.push_back(*it2);
	while (it2 != upperTangentRight)
	{
		it2 = CCWNext(rhs, it2);
		hull.push_back(*it2);
	}

	if (deb)
	{
		deb->drawSegment(*upperTangentLeft, *upperTangentRight, 5.0f, cocos2d::Color4F::YELLOW);
		deb->drawSegment(*lowerTangentLeft, *lowerTangentRight, 5.0f, cocos2d::Color4F::MAGENTA);
	}

}

void GetConvexHull_DAC(std::vector<cocos2d::Vec2> const& set, std::vector<cocos2d::Vec2>& outHull, cocos2d::DrawNode* deb = nullptr)
{
	if (set.empty())
	{
		return;
	}

	if (set.size() == 1)
	{
		outHull.push_back(set.front());
		return;
	}

	if (set.size() <= 5)
	{
		GetConvexHull_Jarvis(set, outHull);
		return;
	}

	static std::random_device rd;
	static std::mt19937 mte(rd());
	std::uniform_int_distribution<std::size_t> indDist(0, set.size() - 1);

	auto pivot = set[indDist(mte)];

	std::vector<cocos2d::Vec2> leftSubset;
	leftSubset.reserve(set.size());
	
	std::copy_if(set.cbegin(), set.cend(), std::back_inserter(leftSubset), [pivot](cocos2d::Vec2 const point) { return point.x <= pivot.x; });

	if (leftSubset.size() < 3 || leftSubset.size() > (set.size() - 3))
	{
		GetConvexHull_Jarvis(set, outHull);
		return;
	}

	std::vector<cocos2d::Vec2> rightSubset;
	rightSubset.reserve(set.size());

	std::copy_if(set.cbegin(), set.cend(), std::back_inserter(rightSubset), [pivot](cocos2d::Vec2 const point) { return point.x > pivot.x; });


	std::vector<cocos2d::Vec2> leftSubHull;
	std::vector<cocos2d::Vec2> rightSubHull;
	leftSubHull.reserve(set.size());
	rightSubHull.reserve(set.size());

	GetConvexHull_DAC(leftSubset, leftSubHull, deb);
	GetConvexHull_DAC(rightSubset, rightSubHull, deb);

	if (deb)
	{
		for (uint32_t i = 0; i < leftSubHull.size(); ++i)
		{
			const cocos2d::Vec2 nextPoint = ((i + 1) == leftSubHull.size()) ? leftSubHull[0] : leftSubHull[i + 1];

			deb->drawSegment(leftSubHull[i], nextPoint, 5.0f, cocos2d::Color4F::GREEN);
		}

		for (uint32_t i = 0; i < rightSubHull.size(); ++i)
		{
			const cocos2d::Vec2 nextPoint = ((i + 1) == rightSubHull.size()) ? rightSubHull[0] : rightSubHull[i + 1];

			deb->drawSegment(rightSubHull[i], nextPoint, 5.0f, cocos2d::Color4F::BLUE);
		}
	}

	Merge_DAC(leftSubHull, rightSubHull, outHull, deb);

	return;
}

