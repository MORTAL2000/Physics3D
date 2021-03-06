#pragma once

#include <sstream>
#include <queue>

#include "../gui/component.h"
#include "../physics/profiling.h"
#include "../physics/math/linalg/largeMatrix.h"

class Font;
class Screen;
struct TreeNode;

extern const Color3 pieColors[30];

struct WeightValue {
	float weight;
	std::string value;
};

struct DataPoint {
	float weight;
	std::string value;
	Color3 color;
	const char* label;
	DataPoint() : color(), weight(0) {}
	DataPoint(float weight, std::string value, Vec3f color, const char* label) : weight(weight), value(value), color(color), label(label) {}
};

struct PieChart {
	const char* title;
	Vec2f piePosition;
	float pieSize;
	std::vector<DataPoint> parts;
	std::string totalValue;

	inline PieChart(const char* title, std::string totalValue, Vec2f piePosition, float pieSize) : title(title), totalValue(totalValue), piePosition(piePosition), pieSize(pieSize) {}
	void renderPie() const;
	void renderText(Font* font) const;
	void add(DataPoint& p);
	float getTotal() const;
};

struct BarChartClassInfo {
	std::string name;
	Color3 color;
};

struct BarChart : public Component {
	const char* title;
	const char** labels;
	BarChartClassInfo* classes;
	LargeMatrix<WeightValue> data;
	std::string totalValue;

	inline BarChart(const char* title, std::string totalValue, const char** labels, BarChartClassInfo* classes, Vec2f chartPosition, Vec2f chartSize, int classCount, int barCount) : 
		title(title), totalValue(totalValue), classes(classes), labels(labels), data(barCount, classCount), Component(chartPosition, chartSize) {}
	void render() override;
	inline Vec2 resize() override { return dimension; };

	float getMaxWeight() const;
};

struct SlidingChartDataSetInfo {
	int size;
	std::string title;
	CircularBuffer<float> data;
	Color color;
	float lineSize;

	float mean;
	float deviation;

	SlidingChartDataSetInfo() : title(""), size(0), mean(0), deviation(1), color(COLOR::ALPHA), lineSize(0), data() {}
	SlidingChartDataSetInfo(const std::string& title, int size, Color color = COLOR::ACCENT, float lineSize = 1.0f) : title(title), size(size), mean(0), deviation(1), color(color), lineSize(lineSize), data(size) {};

	void add(float value);
};

struct SlidingChart : public Component {
	std::string title;
	std::map<std::string, SlidingChartDataSetInfo> dataSets;

	SlidingChart(const std::string& title, const Vec2& position, const Vec2& dimension);

	void add(const std::string& title, float value);
	void add(const SlidingChartDataSetInfo& dataSet);

	SlidingChartDataSetInfo get(const std::string& title);

	void render() override;
	Vec2 resize() override;
};

template<typename T>
class BoundsTree;

void renderTreeStructure(const BoundsTree<Part>& tree, const Color3& treeColor, Vec2f origin, float allottedWidth, const void* selectedObject);

