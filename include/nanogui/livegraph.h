/*
    nanogui/livegraph.h -- Simple graph widget for showing a function plot with data being updated in real time

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <atomic>

#include <nanogui/widget.h>

NAMESPACE_BEGIN(nanogui)

/**
 * \class Graph graph.h nanogui/graph.h
 *
 * \brief Simple graph widget for showing a function plot.
 */
class NANOGUI_EXPORT LiveGraph : public Widget {
public:
	typedef std::vector<std::pair<float, Color>> ColorMap;

	LiveGraph(Widget *parent, const std::string &caption = "Untitled", int bufSize = 1000, Vector2f range= Vector2f(0,1));

    const std::string &caption() const { return mCaption; }
    void setCaption(const std::string &caption) { mCaption = caption; }

    const std::string &header() const { return mHeader; }
    void setHeader(const std::string &header) { mHeader = header; }

    const std::string &footer() const { return mFooter; }
    void setFooter(const std::string &footer) { mFooter = footer; }

    const Color &backgroundColor() const { return mBackgroundColor; }
    void setBackgroundColor(const Color &backgroundColor) { mBackgroundColor = backgroundColor; }

    const Color &foregroundColor() const { return mForegroundColor; }
    void setForegroundColor(const Color &foregroundColor) { mForegroundColor = foregroundColor; }

    const Color &textColor() const { return mTextColor; }
    void setTextColor(const Color &textColor) { mTextColor = textColor; }

    const VectorXf &values() const { return mValues; }
	inline void insertValue(float v) {
		int curWriteHead = mCurWriteHead;
		mValues(curWriteHead) = v;
		curWriteHead = (++curWriteHead) % mValues.rows();
		mCurWriteHead = curWriteHead;
	}

	const Vector2f &range() const { return mRange; }
	Vector2f &range() { return mRange; }
	void setRange(const Vector2f &range) { mRange = range; mFnZero = std::min(std::max(mFnZero, 0.0f), 1.0f); }



    virtual Vector2i preferredSize(NVGcontext *ctx) const override;
    virtual void draw(NVGcontext *ctx) override;

    virtual void save(Serializer &s) const override;
    virtual bool load(Serializer &s) override;

    const ColorMap &colorMap() const { return mColorMap; }
    void setColorMap(const ColorMap &colorMap);

protected:
    std::string mCaption, mHeader, mFooter;
    Color mBackgroundColor, mForegroundColor, mTextColor;
    VectorXf mValues;
	ColorMap mColorMap;
	Vector2f mRange;
	std::atomic<size_t> mCurWriteHead;
	float mFnZero=0;
};

NAMESPACE_END(nanogui)
