#pragma once

template <typename T>
struct Pair
{
	T first;
	T second;
};

template <typename Container>
Pair<typename Container::value_type> findMinMax(const Container& container)
{
	if (container.empty())
		return Pair<typename Container::value_type> {0, 0};

	Pair<typename Container::value_type> minMax {container[0], container[0]};

	for (size_t i = 1, containerSize = container.size(); i < containerSize; ++i)
	{
		const auto item = container[i];
		if (item < minMax.first)
			minMax.first = item;
		else if (item > minMax.second)
			minMax.second = item;
	}

	return minMax;
}