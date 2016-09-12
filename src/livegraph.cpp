/*
src/livegraph.h -- Simple graph widget for showing a function plot with data being updated in real time

NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
The widget drawing code is based on the NanoVG demo application
by Mikko Mononen.

All rights reserved. Use of this source code is governed by a
BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/livegraph.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>

NAMESPACE_BEGIN(nanogui)

LiveGraph::LiveGraph(Widget *parent, const std::string &caption, int bufSize, Vector2f range )
	: Widget(parent),mRange(range), mCaption(caption),mCurWriteHead(0) {
	mValues = VectorXf::Zero(bufSize);
	mBackgroundColor = Color(20, 128);
	mForegroundColor = Color(255, 192, 0, 128);
	mTextColor = Color(240, 192);
}

auto ColorMapCmp = [](const LiveGraph::ColorMap::value_type &a, const LiveGraph::ColorMap::value_type &b)->bool
{
	return a.first < b.first;
};

void LiveGraph::setColorMap(const ColorMap & colorMap)
{
	mColorMap = colorMap;

	// Sort in decreasing order
	std::sort(mColorMap.rbegin(), mColorMap.rend(), ColorMapCmp);

	// Throw out values that are not >= lowerLim
	while (mColorMap.size() != 0 && mColorMap.back().first < mRange[0]) mColorMap.pop_back();

	// Reverse to increasing order
	std::reverse(mColorMap.begin(), mColorMap.end());

	// Throw out values that are not <= upperLim
	while (mColorMap.size() != 0 && mColorMap.back().first > mRange[1]) mColorMap.pop_back();
}

Vector2i LiveGraph::preferredSize(NVGcontext *) const {
	return Vector2i(180, 45);
}

void LiveGraph::draw(NVGcontext *ctx) {
	Widget::draw(ctx);

	nvgBeginPath(ctx);
	nvgRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
	nvgFillColor(ctx, mBackgroundColor);
	nvgFill(ctx);

	size_t curWriteHead = mCurWriteHead;

	std::function<NVGcolor(float)> lookupFn = [&](float)->NVGcolor {return mForegroundColor;};

	if (mColorMap.size() != 0) 
		lookupFn = [&](float v)->NVGcolor {
		size_t j = std::lower_bound(mColorMap.begin(), mColorMap.end(),
			std::make_pair(v, Color()), ColorMapCmp) - mColorMap.begin();

		ColorMap::value_type cl = (j>0) ? (mColorMap[j - 1]) : (mColorMap.front());
		ColorMap::value_type cu = (j<mColorMap.size()) ? (mColorMap[j]) : (mColorMap.back());

		float m = (cl.first != cu.first) ? ((v - cl.first) / (cu.first - cl.first)) : (0.5);
		return nvgLerpRGBA(cl.second, cu.second, m);
	};

	float nz = std::min(std::max((mFnZero - mRange[0]) / (mRange[1] - mRange[0]), 0.0f), 1.0f);
	float uy = mPos.y() + (1 - nz) * mSize.y();

	auto drawRange = [&](int b, int e)->void {

		for (int i = b; i < e && i < mValues.size(); ++i) {

			float v = mValues[i];
			float nv = std::min(std::max((v - mRange[0]) / (mRange[1] - mRange[0]), 0.0f), 1.0f);
			float ux = float(mPos.x()) + float((i - 1) * mSize.x()) / float(mValues.size() - 1);
			float vx = float(mPos.x()) + float(i * mSize.x()) / float(mValues.size() - 1);
			float vy = mPos.y() + (1 - nv) * mSize.y();

			auto c = lookupFn(v);

			nvgBeginPath(ctx);
			nvgMoveTo(ctx, ux, uy);
			nvgLineTo(ctx, ux, vy);
			nvgLineTo(ctx, vx, vy);
			nvgLineTo(ctx, vx, uy);


			//nvgStrokeColor(ctx, c);
			//nvgStroke(ctx);
			nvgFillColor(ctx, c);
			nvgFill(ctx);

		}
	};
	

	drawRange(1, curWriteHead);
	drawRange(curWriteHead + mValues.size()/10, mValues.size());


	// Vertical Red bar
	nvgBeginPath(ctx);
	nvgRect(ctx, mPos.x() + (float(curWriteHead)/float(mValues.size() - 1))*mSize.x() -2, mPos.y(), 4, mSize.y());
	nvgFillColor(ctx, Color(255,0,0,192));
	nvgFill(ctx);

	// Horizontal X axis
	nvgBeginPath(ctx);
	nvgRect(ctx, mPos.x(), uy, mSize.x(),1);
	nvgFillColor(ctx, Color(192, 192));
	nvgFill(ctx);



	nvgFontFace(ctx, "sans");

	if (!mCaption.empty()) {
		nvgFontSize(ctx, 14.0f);
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFillColor(ctx, mTextColor);
		nvgText(ctx, mPos.x() + 3, mPos.y() + 1, mCaption.c_str(), NULL);
	}

	if (!mHeader.empty()) {
		nvgFontSize(ctx, 18.0f);
		nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
		nvgFillColor(ctx, mTextColor);
		nvgText(ctx, mPos.x() + mSize.x() - 3, mPos.y() + 1, mHeader.c_str(), NULL);
	}

	if (!mFooter.empty()) {
		nvgFontSize(ctx, 15.0f);
		nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
		nvgFillColor(ctx, mTextColor);
		nvgText(ctx, mPos.x() + mSize.x() - 3, mPos.y() + mSize.y() - 1, mFooter.c_str(), NULL);
	}

	nvgBeginPath(ctx);
	nvgRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
	nvgStrokeColor(ctx, Color(100, 255));
	nvgStroke(ctx);
}

void LiveGraph::save(Serializer &s) const {
	Widget::save(s);
	s.set("caption", mCaption);
	s.set("header", mHeader);
	s.set("footer", mFooter);
	s.set("backgroundColor", mBackgroundColor);
	s.set("foregroundColor", mForegroundColor);
	s.set("textColor", mTextColor);
	s.set("values", mValues);
}

bool LiveGraph::load(Serializer &s) {
	if (!Widget::load(s)) return false;
	if (!s.get("caption", mCaption)) return false;
	if (!s.get("header", mHeader)) return false;
	if (!s.get("footer", mFooter)) return false;
	if (!s.get("backgroundColor", mBackgroundColor)) return false;
	if (!s.get("foregroundColor", mForegroundColor)) return false;
	if (!s.get("textColor", mTextColor)) return false;
	if (!s.get("values", mValues)) return false;
	return true;
}

NAMESPACE_END(nanogui)
