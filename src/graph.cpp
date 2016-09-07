/*
    src/graph.cpp -- Simple graph widget for showing a function plot

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/graph.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>

NAMESPACE_BEGIN(nanogui)

Graph::Graph(Widget *parent, const std::string &caption)
    : Widget(parent), mCaption(caption) {
    mBackgroundColor = Color(20, 128);
    mForegroundColor = Color(255, 192, 0, 128);
    mTextColor = Color(240, 192);
}

auto ColorMapCmp = [](const Graph::ColorMap::value_type &a, const Graph::ColorMap::value_type &b)->bool 
{
	return a.first < b.first;
};

void Graph::setColorMap(const ColorMap & colorMap)
{
	mColorMap = colorMap;

	// Sort in decreasing order
	std::sort(mColorMap.rbegin(), mColorMap.rend(), ColorMapCmp);

	// Throw out values that are not >= 0
	while (mColorMap.size() !=0 && mColorMap.back().first < 0.0) mColorMap.pop_back();

	// Reverse to increasing order
	std::reverse(mColorMap.begin(), mColorMap.end());

	// Throw out values that are not <= 1
	while (mColorMap.size() != 0 && mColorMap.back().first > 1.0) mColorMap.pop_back();
}

Vector2i Graph::preferredSize(NVGcontext *) const {
    return Vector2i(180, 45);
}

void Graph::draw(NVGcontext *ctx) {
    Widget::draw(ctx);

	nvgBeginPath(ctx);
	nvgRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
	nvgFillColor(ctx, mBackgroundColor);
	nvgFill(ctx);

	if (mValues.size() >= 2) {

		if (mColorMap.size() != 0) {

			float u = mValues[0];
			float ux = mPos.x();
			float uy = mPos.y() + (1 - u) * mSize.y();


			for (size_t i = 1; i < (size_t)mValues.size(); i++) {

				float v = mValues[i];
				float vx = float(mPos.x()) + float(i * mSize.x()) / float(mValues.size() - 1);
				float vy = mPos.y() + (1 - v) * mSize.y();

				float s = (u + v) / 2;
				size_t j = std::lower_bound(mColorMap.begin(), mColorMap.end(), 
					std::make_pair(s,Color()), ColorMapCmp) - mColorMap.begin();

				ColorMap::value_type cl = (j>0) ? (mColorMap[j - 1]) : (mColorMap.front());
				ColorMap::value_type cu = (j<mColorMap.size()) ? (mColorMap[j]) : (mColorMap.back());

				float m = (cl.first != cu.first) ? ((s - cl.first) / (cu.first - cl.first)) : (0.5);
				auto c = nvgLerpRGBA(cl.second, cu.second, m);

				nvgBeginPath(ctx);
				nvgMoveTo(ctx, ux, mPos.y() + mSize.y());
				nvgLineTo(ctx, ux, uy);
				nvgLineTo(ctx, vx, vy);
				nvgMoveTo(ctx, vx, mPos.y() + mSize.y());


				nvgStrokeColor(ctx, c);
				nvgStroke(ctx);
				nvgFillColor(ctx, c);
				nvgFill(ctx);

				u = v; ux = vx; uy = vy;
			}

		}
		else {
			nvgBeginPath(ctx);
			nvgMoveTo(ctx, mPos.x(), mPos.y() + mSize.y());
			for (size_t i = 0; i < (size_t)mValues.size(); i++) {
				float value = mValues[i];
				float vx = mPos.x() + i * mSize.x() / (float)(mValues.size() - 1);
				float vy = mPos.y() + (1 - value) * mSize.y();
				nvgLineTo(ctx, vx, vy);
			}

			nvgLineTo(ctx, mPos.x() + mSize.x(), mPos.y() + mSize.y());
			nvgStrokeColor(ctx, Color(100, 255));
			nvgStroke(ctx);
			nvgFillColor(ctx, mForegroundColor);
			nvgFill(ctx);
		}
	}

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

void Graph::save(Serializer &s) const {
    Widget::save(s);
    s.set("caption", mCaption);
    s.set("header", mHeader);
    s.set("footer", mFooter);
    s.set("backgroundColor", mBackgroundColor);
    s.set("foregroundColor", mForegroundColor);
    s.set("textColor", mTextColor);
    s.set("values", mValues);
}

bool Graph::load(Serializer &s) {
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
