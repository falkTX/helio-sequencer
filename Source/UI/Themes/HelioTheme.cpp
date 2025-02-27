/*
    This file is part of Helio music sequencer.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "HelioTheme.h"
#include "SequencerSidebarLeft.h"
#include "ColourIDs.h"
#include "Config.h"
#include "SerializationKeys.h"

#include "PageBackgroundA.h"
#include "PageBackgroundB.h"
#include "PanelBackground.h"

#if PLATFORM_DESKTOP
#   define SCROLLBAR_WIDTH (17)
#elif PLATFORM_MOBILE
#   define SCROLLBAR_WIDTH (50)
#endif

HelioTheme::HelioTheme() :
    backgroundNoise(ImageCache::getFromMemory(BinaryData::noise_png, BinaryData::noise_pngSize)) {}

HelioTheme &HelioTheme::getCurrentTheme() noexcept
{
    // This assumes the app have set an instance of HelioTheme as default look-and-feel
    return static_cast<HelioTheme &>(LookAndFeel::getDefaultLookAndFeel());
}

static constexpr auto noiseAlpha = 0.0125f;

void HelioTheme::drawNoise(Component *target, Graphics &g, float alphaMultiply /*= 1.f*/)
{
    g.setTiledImageFill(getCurrentTheme().backgroundNoise, 0, 0, noiseAlpha * alphaMultiply);
    g.fillRect(0, 0, target->getWidth(), target->getHeight());
}

void HelioTheme::drawNoise(Graphics &g, float alphaMultiply /*= 1.f*/) const
{
    g.setTiledImageFill(this->backgroundNoise, 0, 0, noiseAlpha * alphaMultiply);
    g.fillRect(0, 0, g.getClipBounds().getWidth(), g.getClipBounds().getHeight());
}

void HelioTheme::drawNoise(const HelioTheme &theme, Graphics &g, float alphaMultiply /*= 1.f*/)
{
    theme.drawNoise(g, alphaMultiply);
}

void HelioTheme::drawNoiseWithin(Rectangle<int> bounds, Graphics &g, float alphaMultiply /*= 1.f*/)
{
    g.setTiledImageFill(getCurrentTheme().backgroundNoise, 0, 0, noiseAlpha * alphaMultiply);
    g.fillRect(bounds);
}

//===----------------------------------------------------------------------===//
// Frames/lines helpers
//===----------------------------------------------------------------------===//

void HelioTheme::drawFrame(Graphics &g, int width, int height,
    float lightAlphaMultiplier, float darkAlphaMultiplier)
{
    g.setColour(findDefaultColour(ColourIDs::Common::borderLineDark).
        withMultipliedAlpha(darkAlphaMultiplier));

    g.fillRect(1, 0, width - 2, 2);
    g.fillRect(1, height - 2, width - 2, 2);
    g.fillRect(0, 1, 2, height - 2);
    g.fillRect(width - 2, 1, 2, height - 2);

    g.setColour(findDefaultColour(ColourIDs::Common::borderLineLight).
        withMultipliedAlpha(lightAlphaMultiplier));

    g.fillRect(2, 1, width - 4, 1);
    g.fillRect(2, height - 2, width - 4, 1);
    g.fillRect(1, 2, 1, height - 4);
    g.fillRect(width - 2, 2, 1, height - 4);
}

void HelioTheme::drawDashedFrame(Graphics &g, const Rectangle<int> &area, int dashLength /*= 4*/)
{
    for (int i = area.getX() + 1; i < area.getWidth() - 1; i += (dashLength * 2))
    {
        g.fillRect(i, area.getY(), dashLength, 1);
        g.fillRect(i, area.getHeight() - 1, dashLength, 1);
    }

    for (int i = area.getY() + 1; i < area.getHeight() - 1; i += (dashLength * 2))
    {
        g.fillRect(area.getX(), i, 1, dashLength);
        g.fillRect(area.getWidth() - 1, i, 1, dashLength);
    }
}

void HelioTheme::drawDashedHorizontalLine(Graphics &g, float x, float y, float width, float dashLength)
{
    for (; x < width - dashLength; x += dashLength * 2.f)
    {
        g.fillRect(x, y, dashLength, 1.f);
    }

    if (width > dashLength)
    {
        g.fillRect(x, y, jmax(0.f, 1.f + width - x), 1.f);
    }
}

void HelioTheme::drawDashedHorizontalLine2(Graphics &g, float x, float y, float width, float dashLength)
{
    for (; x < width - dashLength; x += dashLength * 2.f)
    {
        g.fillRect(x + 1.f, y, dashLength, 1.f);
        g.fillRect(x, y + 1.f, dashLength, 1.f);
    }

    if (width > dashLength)
    {
        g.fillRect(x + 1.f, y, jmax(0.f, 1.f + width - x - 2.f), 1.f);
        g.fillRect(x, y + 1.f, jmax(0.f, 1.f + width - x), 1.f);
    }
}

void HelioTheme::drawDashedHorizontalLine3(Graphics &g, float x, float y, float width, float dashLength)
{
    for (; x < width - dashLength; x += dashLength * 2.f)
    {
        g.fillRect(x + 2.f, y, dashLength, 1.f);
        g.fillRect(x + 1.f, y + 1.f, dashLength, 1.f);
        g.fillRect(x, y + 2.f, dashLength, 1.f);
    }

    if (width > dashLength)
    {
        g.fillRect(x + 2.f, y, jmax(0.f, 1.f + width - x - 4.f), 1.f);
        g.fillRect(x + 1.f, y + 1.f, jmax(0.f, 1.f + width - x - 2.f), 1.f);
        g.fillRect(x, y + 2.f, jmax(0.f, 1.f + width - x), 1.f);
    }
}

void HelioTheme::drawDashedVerticalLine(Graphics &g, float x, float y, float height, float dashLength)
{
    for (; y < height - dashLength; y += dashLength * 2.f)
    {
        g.fillRect(x, y, 1.f, dashLength);
    }

    if (height > dashLength)
    {
        g.fillRect(x, y, 1.f, jmax(0.f, 1.f + height - y));
    }
}

Typeface::Ptr HelioTheme::getTypefaceForFont(const Font &font)
{
#if PLATFORM_DESKTOP
    if (font.getTypefaceName() == Font::getDefaultSansSerifFontName() ||
        font.getTypefaceName() == Font::getDefaultSerifFontName())
    {
        return this->textTypefaceCache;
    }
#endif

    return Font::getDefaultTypefaceForFont(font);
}

void HelioTheme::drawStretchableLayoutResizerBar(Graphics &g,
        int w, int h, bool isVerticalBar,
        bool isMouseOver, bool isMouseDragging) {}

//===----------------------------------------------------------------------===//
// Text Editor
//===----------------------------------------------------------------------===//

void HelioTheme::fillTextEditorBackground(Graphics &g, int w, int h, TextEditor &ed)
{
    g.setColour(findDefaultColour(TextEditor::backgroundColourId));
    g.fillRect(1, 1, w - 2, h - 2);
    g.setColour(findDefaultColour(TextEditor::outlineColourId));
    g.drawVerticalLine(0, 1.f, h - 1.f);
    g.drawVerticalLine(w - 1, 1.f, h - 1.f);
    g.drawHorizontalLine(0, 1.f, w - 1.f);
    g.drawHorizontalLine(h - 1, 1.f, w - 1.f);
}

//===----------------------------------------------------------------------===//
// Labels
//===----------------------------------------------------------------------===//

Font HelioTheme::getLabelFont(Label &label)
{
#if PLATFORM_DESKTOP
    return label.getFont();
#elif PLATFORM_MOBILE
    // For whatever reason, fonts on the mobiles look way larger
    // than fonts of same height on the desktops:
    auto font = label.getFont();
    font.setHeight(font.getHeight() - 2);
    return font;
#endif
}

void HelioTheme::drawLabel(Graphics &g, Label &label)
{
    this->drawLabel(g, label, 0);
}

#if PLATFORM_DESKTOP
#   define SMOOTH_RENDERED_FONT 1
#endif

// Label rendering is one of the most time-consuming bottlenecks in the app.
// Calling this method too often should be avoided at any costs:
// relatively small labels that are always on the screen (like headline titles,
// annotations, key/time signatures, track headers) should be set cached to images,
// and they also should have fixed sizes (not dependent on a container component's size)
// so that resizing a parent won't force resizing, re-rendering and re-caching a label.
// Such labels should be re-rendered only when their content changes.

void HelioTheme::drawLabel(Graphics &g, Label &label, juce_wchar passwordCharacter)
{
    if (! label.isBeingEdited())
    {
        const Font font(this->getLabelFont(label));
        const String textToDraw = (passwordCharacter != 0) ?
            String::repeatedString(String::charToString(passwordCharacter), label.getText().length()) :
            label.getText();

        // Try to guess the right max number of lines depending on label height and font height:
        const int maxLines = int(float(label.getHeight()) / font.getHeight());

        // using label.findColour, not findDefaultColour, as it is actually overridden in some places:
        const Colour colour = label.findColour(Label::textColourId);

#if SMOOTH_RENDERED_FONT

        Path textPath;
        GlyphArrangement glyphs;

        const auto textArea =
            label.getBorderSize().subtractedFrom(label.getLocalBounds()).toFloat();

        glyphs.addFittedText(font,
            textToDraw,
            textArea.getX(),
            textArea.getY(),
            textArea.getWidth(),
            textArea.getHeight(),
            label.getJustificationType(),
            maxLines,
            1.0);

        glyphs.createPath(textPath);

        // todo play with outlines for some cached labels?
        //g.setColour(findDefaultColour(Label::outlineColourId));
        //g.strokePath(textPath, PathStrokeType(2.0f));

        g.setColour(colour);
        g.fillPath(textPath);

#else

        const auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds());

        // I really have no idea why drawFittedText on both iOS and Android renders the font
        // slightly above the desired position, so I'm simply adding some offset:
        const int yOffsetHack = 1;

        g.setFont(font);
        g.setColour(colour);
        g.drawFittedText(textToDraw,
            textArea.getX(),
            textArea.getY() + yOffsetHack,
            textArea.getWidth(),
            textArea.getHeight() + yOffsetHack,
            label.getJustificationType(),
            maxLines,
            1.0);

#endif
    }
}

//===----------------------------------------------------------------------===//
// Button
//===----------------------------------------------------------------------===//

Font HelioTheme::getTextButtonFont(TextButton &button, int buttonHeight)
{
#if PLATFORM_DESKTOP
    return { Globals::UI::Fonts::L };
#elif PLATFORM_MOBILE
    return { Globals::UI::Fonts::M };
#endif
}

void HelioTheme::drawButtonText(Graphics &g, TextButton &button,
                                bool isMouseOverButton, bool isButtonDown)
{
    const auto font = getTextButtonFont(button, button.getHeight());
    g.setFont(font);
    g.setColour(findDefaultColour(TextButton::buttonColourId).contrasting()
        .withMultipliedAlpha(button.isEnabled() ? 0.9f : 0.4f));
    
    const int yIndent = jmin(4, button.proportionOfHeight(0.3f));
    const int yHeight = (button.getHeight() - (yIndent * 2));
    const int cornerSize = jmin(button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = int(font.getHeight() * 0.5f);
    const int leftIndent = fontHeight;
    const int rightIndent = jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));

    g.setColour(findDefaultColour(TextButton::textColourOnId)
        .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

    g.drawFittedText(button.getButtonText(),
        leftIndent,
        yIndent,
        button.getWidth() - leftIndent - rightIndent,
        yHeight,
        Justification::centred, 4, 1.f);
}

void HelioTheme::drawButtonBackground(Graphics &g, Button &button,
                                      const Colour &backgroundColour,
                                      bool isMouseOverButton, bool isButtonDown)
{
    const auto baseColour = backgroundColour
        .withMultipliedAlpha(button.isEnabled() ? 0.75f : 0.3f);

    const auto width = float(button.getWidth());
    const auto height = float(button.getHeight());

    if (width > 0 && height > 0)
    {
        const float cornerSize = 1.f;

        Path outline;
        outline.addRoundedRectangle(0.f, 0.f,
            width, height, cornerSize, cornerSize,
            true, true, true, true);

        g.setGradientFill(ColourGradient(
            baseColour.darker(0.1f), 0.f, height / 2 - 2,
            baseColour.darker(0.2f), 0.f, height / 2 + 2, false));

        g.fillPath(outline);

        if (isButtonDown || isMouseOverButton)
        {
            g.setColour(baseColour.brighter(isButtonDown ? 0.1f : 0.01f));
            g.fillPath(outline);
        }

        constexpr auto outlineAlpha = 0.05f;

        g.setColour(Colours::white.withAlpha(outlineAlpha));
        g.strokePath(outline, PathStrokeType(1.f));

        g.setColour(Colours::black.withAlpha(outlineAlpha));
        g.strokePath(outline, PathStrokeType(1.f),
            AffineTransform::translation(0.f, 1.f).scaled(1.f, (height + 2.f) / height));
    }
}

//===----------------------------------------------------------------------===//
// Tables
//===----------------------------------------------------------------------===//

void HelioTheme::drawTableHeaderBackground(Graphics &g, TableHeaderComponent &header) {}

void HelioTheme::drawTableHeaderColumn(Graphics &g,
    TableHeaderComponent &header, const String &columnName,
    int columnId, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags)
{
    const auto highlightColour = findDefaultColour(TableHeaderComponent::highlightColourId);

    if (isMouseDown)
    {
        g.fillAll(highlightColour);
    }
    else if (isMouseOver)
    {
        g.fillAll(highlightColour.withMultipliedAlpha(0.625f));
    }

    Rectangle<int> area(width, height);
    area.reduce(4, 0);

    g.setColour(findDefaultColour(TableHeaderComponent::textColourId));
    if ((columnFlags & (TableHeaderComponent::sortedForwards | TableHeaderComponent::sortedBackwards)) != 0)
    {
        Path sortArrow;
        sortArrow.addTriangle(0.f, 0.f,
            0.5f, (columnFlags & TableHeaderComponent::sortedForwards) != 0 ? -0.8f : 0.8f,
            1.f, 0.f);

        g.fillPath(sortArrow,
            sortArrow.getTransformToScaleToFit(area.removeFromRight(height / 2)
                .reduced(6).toFloat(), true));
    }

    g.setFont(Globals::UI::Fonts::M);
    g.drawFittedText(columnName, area, columnId == 1 ? // a nasty hack, only used in AudioPluginsListComponent
        Justification::centredRight : Justification::centredLeft, 1);
}

//===----------------------------------------------------------------------===//
// Scrollbars
//===----------------------------------------------------------------------===//

int HelioTheme::getDefaultScrollbarWidth()
{
    return SCROLLBAR_WIDTH;
}

void HelioTheme::drawScrollbar(Graphics &g, ScrollBar &scrollbar,
                               int x, int y, int width, int height,
                               bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                               bool isMouseOver, bool isMouseDown)
{
    Path thumbPath;

    if (thumbSize > 0)
    {
        const float thumbIndent = (isScrollbarVertical ? width : height) * 0.25f;
        const float thumbIndentx2 = thumbIndent * 2.0f;

        if (isScrollbarVertical)
        {
            thumbPath.addRoundedRectangle(x + thumbIndent, thumbStartPosition + thumbIndent,
                                          width - thumbIndentx2, thumbSize - thumbIndentx2,
                                          (width - thumbIndentx2) * 0.5f);
        }
        else
        {
            thumbPath.addRoundedRectangle(thumbStartPosition + thumbIndent, y + thumbIndent,
                                          thumbSize - thumbIndentx2, height - thumbIndentx2,
                                          (height - thumbIndentx2) * 0.5f);
        }
    }

    Colour thumbCol(findDefaultColour(ScrollBar::thumbColourId));

    if (isMouseOver || isMouseDown)
    { thumbCol = thumbCol.withMultipliedAlpha(0.5f); }
    else
    { thumbCol = thumbCol.withMultipliedAlpha(0.1f); }

    g.setColour(thumbCol);
    g.fillPath(thumbPath);

    g.setColour(thumbCol.contrasting((isMouseOver  || isMouseDown) ? 0.2f : 0.1f));
    g.strokePath(thumbPath, PathStrokeType(1.0f));
}

//===----------------------------------------------------------------------===//
// Sliders
//===----------------------------------------------------------------------===//

void HelioTheme::drawRotarySlider(Graphics &g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle, Slider &slider)
{
    const auto fill = findDefaultColour(Slider::rotarySliderFillColourId);
    const auto outline = findDefaultColour(Slider::rotarySliderOutlineColourId);
    
    const auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(8);
    const auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto lineW = jmin(8.0f, radius * 0.25f);
    const auto arcRadius = radius - lineW * 0.5f;

    if (slider.isEnabled())
    {
        Path fullArc;
        fullArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
            arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

        g.setColour(outline);
        g.strokePath(fullArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));

        Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
            arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);

        g.setColour(fill);
        g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }
}

//===----------------------------------------------------------------------===//
// Window
//===----------------------------------------------------------------------===//

void HelioTheme::drawCornerResizer(Graphics& g, int w, int h,
    bool /*isMouseOver*/, bool /*isMouseDragging*/)
{
    const float lineThickness = jmin(w, h) * 0.05f;
    const Colour col1(this->findColour(ResizableWindow::backgroundColourId).darker(0.2f));
    const Colour col2(this->findColour(ResizableWindow::backgroundColourId).brighter(0.04f));

    for (float i = 0.8f; i > 0.2f; i -= 0.25f)
    {
        g.setColour(col1);

        g.drawLine(w * i,
            h + 1.0f,
            w + 1.0f,
            h * i,
            lineThickness);

        g.setColour(col2);

        g.drawLine(w * i + lineThickness,
            h + 1.0f,
            w + 1.0f,
            h * i + lineThickness,
            lineThickness);
    }
}

void HelioTheme::drawResizableFrame(Graphics &g, int w, int h, const BorderSize<int> &border)
{
    if (!border.isEmpty())
    {
        const Rectangle<int> fullSize(0, 0, w, h);
        const Rectangle<int> centreArea(border.subtractedFrom(fullSize));

        g.saveState();

        g.excludeClipRegion(centreArea);

        g.setColour(this->findColour(ResizableWindow::backgroundColourId));
        g.drawRect(fullSize, 5);

        g.restoreState();
    }
}

class HelioWindowButton final : public Button
{
public:

    explicit HelioWindowButton(const Path &shape) noexcept :
        Button({}),
        shape(shape) {}

    void paintButton(Graphics &g, bool isMouseOverButton, bool isButtonDown) override
    {
        float alpha = isMouseOverButton ? (isButtonDown ? 1.0f : 0.8f) : 0.6f;
        if (!this->isEnabled())
        {
            alpha *= 0.5f;
        }

        float x = 0, y = 0, diam;

        if (this->getWidth() < this->getHeight())
        {
            diam = (float)this->getWidth();
            y = (this->getHeight() - this->getWidth()) * 0.5f;
        }
        else
        {
            diam = (float)this->getHeight();
            y = (this->getWidth() - this->getHeight()) * 0.5f;
        }

        x += diam * 0.05f;
        y += diam * 0.05f;
        diam *= 0.9f;

        const Colour colour(findDefaultColour(TextButton::textColourOnId));

        if (isMouseOverButton)
        {
            g.setColour(colour.withAlpha(alpha * 0.05f));
            g.fillAll();
        }

        const AffineTransform t(this->shape.getTransformToScaleToFit(x + diam * 0.3f,
            y + diam * 0.3f, diam * 0.4f, diam * 0.4f, true));

        g.setColour(colour);
        g.fillPath(this->shape, t);
    }

private:

    const Path shape;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelioWindowButton)
};

void HelioTheme::drawDocumentWindowTitleBar(DocumentWindow &window,
    Graphics &g, int w, int h,
    int titleSpaceX, int titleSpaceW,
    const Image *icon,
    bool drawTitleTextOnLeft)
{
#if PLATFORM_DESKTOP
    if (window.isUsingNativeTitleBar())
    {
        return;
    }

    const auto &theme = HelioTheme::getCurrentTheme();
    g.setFillType({ theme.getPageBackgroundA(), {} });
    g.fillRect(0, 0, w, h);

    g.setColour(findDefaultColour(ColourIDs::Common::borderLineLight));
    g.fillRect(0, h - 2, w, 1);
    g.setColour(findDefaultColour(ColourIDs::Common::borderLineDark));
    g.fillRect(0, h - 1, w, 1);
    g.setColour(findDefaultColour(ColourIDs::Common::borderLineLight).withMultipliedAlpha(0.35f));
    g.fillRect(0, 0, w, 1);

    const auto uiScaleFactor = App::Config().getUiFlags()->getUiScaleFactor();
    const Font font(Globals::UI::Fonts::S * uiScaleFactor, Font::plain);
    g.setFont(font);

    static const String title = "helio " + App::getAppReadableVersion();
    const auto textWidth = font.getStringWidth(title);
    g.setColour(findDefaultColour(Label::textColourId).withAlpha(0.2f));
    g.drawText(title, titleSpaceX, 0, w - textWidth - 24, h - 3, Justification::centredRight, true);
#endif
}

Button *HelioTheme::createDocumentWindowButton(int buttonType)
{
    Path shape;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment(Line<float>(0.0f, 0.0f, 1.0f, 1.0f), 0.05f);
        shape.addLineSegment(Line<float>(1.0f, 0.0f, 0.0f, 1.0f), 0.05f);
        return new HelioWindowButton(shape);
    }
    if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment(Line<float>(0.0f, 0.0f, 0.00001f, 0.0f), 0.00001f);
        shape.addLineSegment(Line<float>(0.0f, 0.55f, 1.0f, 0.55f), 0.05f);
        return new HelioWindowButton(shape);
    }
    else if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment(Line<float>(0.0f, 0.0f, 0.0f, 0.8f), 0.05f);
        shape.addLineSegment(Line<float>(0.0f, 0.8f, 1.0f, 0.8f), 0.05f);
        shape.addLineSegment(Line<float>(1.0f, 0.8f, 1.0f, 0.0f), 0.05f);
        shape.addLineSegment(Line<float>(1.0f, 0.0f, 0.0f, 0.0f), 0.05f);
        return new HelioWindowButton(shape);
    }

    jassertfalse;
    return nullptr;
}

void HelioTheme::positionDocumentWindowButtons(DocumentWindow &,
        int titleBarX, int titleBarY,
        int titleBarW, int titleBarH,
        Button *minimiseButton,
        Button *maximiseButton,
        Button *closeButton,
        bool positionTitleBarButtonsOnLeft)
{
    const int buttonSize = int(23 * App::Config().getUiFlags()->getUiScaleFactor());
    const int y = ((titleBarH - titleBarY) / 2) - (buttonSize / 2) - 1;
    int x = titleBarX + titleBarW - buttonSize - buttonSize / 4;

    if (closeButton != nullptr)
    {
        closeButton->setBounds(x, y, buttonSize, buttonSize);
        x += -(buttonSize + buttonSize / 4);
    }

    if (maximiseButton != nullptr)
    {
        maximiseButton->setBounds(x, y, buttonSize, buttonSize);
        x += -(buttonSize + buttonSize / 4);
    }

    if (minimiseButton != nullptr)
    {
        minimiseButton->setBounds(x, y, buttonSize, buttonSize);
    }
}

void HelioTheme::initResources()
{
    Icons::initBuiltInImages();

#if PLATFORM_DESKTOP

    if (App::Config().containsProperty(Serialization::Config::lastUsedFont))
    {
        const String lastUsedFontName = App::Config().getProperty(Serialization::Config::lastUsedFont);
        this->textTypefaceCache = Typeface::createSystemTypefaceFor(Font(lastUsedFontName, 0, Font::plain));
        return;
    }

#if DEBUG
    const auto scanStartTime = Time::getMillisecondCounter();
#endif

    // using Font::findAllTypefaceNames instead of the slower
    // Font::findFonts which seems to cause freezes on some systems
    const auto systemFonts = Font::findAllTypefaceNames();

    DBG("Found " + String(systemFonts.size()) + " fonts in " +
        String(Time::getMillisecondCounter() - scanStartTime) + " ms");

    const auto userLanguage = SystemStats::getUserLanguage().toLowerCase().substring(0, 2);
    auto preferredFontNames = StringArray("Noto Sans", "Noto Sans UI", "Source Han Sans");

    if (userLanguage == "kr")
    {
        preferredFontNames.add("Dotum");
    }
    else if (userLanguage == "zh" || userLanguage == "ja")
    {
        preferredFontNames.addArray({ "YeHei", "Hei", "Heiti SC" });
    }

#if JUCE_LINUX
    preferredFontNames.add("Ubuntu");
    preferredFontNames.add("Liberation Sans");
#endif

    String pickedFontName;
    const String perfectlyFineFontName = "Noto Sans CJK";
    for (const auto &fontName : systemFonts)
    {
        if (preferredFontNames.contains(fontName))
        {
            pickedFontName = fontName;
        }
        else if (fontName.startsWithIgnoreCase(perfectlyFineFontName))
        {
            pickedFontName = fontName;
            break;
        }
    }

    if (pickedFontName.isNotEmpty())
    {
        this->textTypefaceCache = Typeface::createSystemTypefaceFor({ pickedFontName, 0, 0 });
    }
    else
    {
        // Verdana on Windows, Bitstream Vera Sans or something on Linux, Lucida Grande on macOS:
        this->textTypefaceCache = Font::getDefaultTypefaceForFont({ Font::getDefaultSansSerifFontName(), 0, 0 });
    }

    DBG("Using font: " + this->textTypefaceCache->getName());
    App::Config().setProperty(Serialization::Config::lastUsedFont, this->textTypefaceCache->getName());

#endif
}

void HelioTheme::updateFont(const Font &font)
{
#if PLATFORM_DESKTOP

    Typeface::clearTypefaceCache();
    this->textTypefaceCache = Typeface::createSystemTypefaceFor(font);
    App::Config().setProperty(Serialization::Config::lastUsedFont, font.getTypefaceName());

#endif
}

void HelioTheme::initColours(const ::ColourScheme::Ptr s)
{
    const auto textColour = s->getTextColour();

    // JUCE component colour id's:

    // Sliders
    this->setColour(Slider::rotarySliderOutlineColourId, textColour.contrasting(0.9f).withMultipliedAlpha(0.5f));
    this->setColour(Slider::rotarySliderFillColourId, textColour);
    this->setColour(Slider::thumbColourId, textColour);
    this->setColour(Slider::trackColourId, textColour.withMultipliedAlpha(0.5f));

    // Labels
    this->setColour(Label::textColourId, textColour);
    this->setColour(Label::outlineColourId, textColour.contrasting());

    // MainWindow
    this->setColour(ResizableWindow::backgroundColourId, s->getPageFillColour().brighter(0.045f));
    this->setColour(ScrollBar::backgroundColourId, Colours::transparentBlack);
    this->setColour(ScrollBar::thumbColourId, s->getButtonFillColour().withAlpha(1.f));

    // TextButton
    this->setColour(TextButton::buttonColourId, s->getButtonFillColour());
    this->setColour(TextButton::buttonOnColourId, s->getFrameBorderColour());
    this->setColour(TextButton::textColourOnId, textColour.withMultipliedAlpha(0.75f));
    this->setColour(TextButton::textColourOffId, textColour.withMultipliedAlpha(0.5f));

    // TextEditor
    this->setColour(TextEditor::textColourId, textColour);
    this->setColour(TextEditor::backgroundColourId, s->getPageFillColour().darker(0.055f));
    this->setColour(TextEditor::outlineColourId, s->getFrameBorderColour().withAlpha(0.075f));
    this->setColour(TextEditor::highlightedTextColourId, textColour);
    this->setColour(TextEditor::focusedOutlineColourId, textColour.contrasting().withAlpha(0.2f));
    this->setColour(TextEditor::shadowColourId, s->getPageFillColour().darker(0.05f));
    this->setColour(TextEditor::highlightColourId, textColour.contrasting().withAlpha(0.25f));
    this->setColour(CaretComponent::caretColourId, textColour.withAlpha(0.35f));

    // TableListBox
    this->setColour(ListBox::textColourId, textColour);
    this->setColour(ListBox::backgroundColourId, Colours::transparentBlack);
    this->setColour(TableHeaderComponent::backgroundColourId, Colours::transparentBlack);
    this->setColour(TableHeaderComponent::outlineColourId, s->getFrameBorderColour().withAlpha(0.05f));
    this->setColour(TableHeaderComponent::highlightColourId, s->getPageFillColour().brighter(0.04f));
    this->setColour(TableHeaderComponent::textColourId, textColour.withMultipliedAlpha(0.75f));

    // Check boxes, radio buttons
    this->setColour(ToggleButton::textColourId, textColour);
    this->setColour(ToggleButton::tickColourId, textColour);
    this->setColour(ToggleButton::tickDisabledColourId, textColour.withMultipliedAlpha(0.65f));

    // Helio colours:

    // Lasso
    this->setColour(ColourIDs::SelectionComponent::fill, s->getLassoFillColour().withAlpha(0.2f));
    this->setColour(ColourIDs::SelectionComponent::outline, s->getLassoBorderColour().withAlpha(0.75f));
    // Similar things for the roll header tools
    this->setColour(ColourIDs::RollHeader::selection, s->getLassoBorderColour().withAlpha(0.8f));
    this->setColour(ColourIDs::RollHeader::soundProbe, s->getLassoBorderColour().withMultipliedBrightness(1.1f).withAlpha(0.75f));
    this->setColour(ColourIDs::RollHeader::timeDistance, textColour.withAlpha(0.4f));

    // A hack for icon base colors
    this->setColour(ColourIDs::Icons::fill, s->getIconBaseColour());
    this->setColour(ColourIDs::Icons::shadow, s->getIconShadowColour());

    // Backgrounds
    this->setColour(ColourIDs::Backgrounds::pageFillA, s->getPageFillColour());
    this->setColour(ColourIDs::Backgrounds::pageFillB, s->getPageFillColour().darker(0.025f));
    this->setColour(ColourIDs::Backgrounds::sidebarFill, s->getSidebarFillColour());
    this->setColour(ColourIDs::Backgrounds::headlineFill, s->getHeadlineFillColour());
    this->setColour(ColourIDs::Backgrounds::dialogFill, s->getDialogFillColour());

    this->setColour(ColourIDs::Tooltip::messageFill, Colours::black.withAlpha(0.75f));
    this->setColour(ColourIDs::Tooltip::okIconFill, Colours::black.withAlpha(0.5f));
    this->setColour(ColourIDs::Tooltip::okIconForeground, Colours::white.withAlpha(0.75f));
    this->setColour(ColourIDs::Tooltip::failIconFill, Colours::black.withAlpha(0.5f));
    this->setColour(ColourIDs::Tooltip::failIconForeground, Colours::white.withAlpha(0.75f));

    this->setColour(ColourIDs::Panel::border, s->getFrameBorderColour().withAlpha(0.225f));

    this->setColour(ColourIDs::TrackScroller::borderLineDark, s->getPageFillColour().darker(0.4f));
    this->setColour(ColourIDs::TrackScroller::borderLineLight, Colours::white.withAlpha(0.055f));
    this->setColour(ColourIDs::TrackScroller::screenRangeFill, s->getIconBaseColour().withMultipliedAlpha(0.6f));
    this->setColour(ColourIDs::TrackScroller::scrollerFill, s->getIconBaseColour().withMultipliedAlpha(0.55f));

    // InstrumentEditor
    this->setColour(ColourIDs::Instrument::midiIn, textColour.contrasting().withAlpha(0.1f));
    this->setColour(ColourIDs::Instrument::midiOut, textColour.contrasting().withAlpha(0.1f));
    this->setColour(ColourIDs::Instrument::audioIn, textColour.contrasting().withAlpha(0.15f));
    this->setColour(ColourIDs::Instrument::audioOut, textColour.contrasting().withAlpha(0.15f));

    this->setColour(ColourIDs::Instrument::midiConnector, textColour.withAlpha(0.15f));
    this->setColour(ColourIDs::Instrument::audioConnector, textColour.contrasting().withAlpha(0.75f));
    this->setColour(ColourIDs::Instrument::pinShadow, textColour.withAlpha(0.1f));
    this->setColour(ColourIDs::Instrument::connectorShadow, textColour.withAlpha(0.25f));

    // Borders
    this->setColour(ColourIDs::Common::borderLineLight, Colours::white.withAlpha(0.0675f));
    this->setColour(ColourIDs::Common::borderLineDark, Colours::black.withAlpha(0.3f));
    this->setColour(ColourIDs::ColourButton::outline, textColour);

    // CallOutBox
    this->setColour(ColourIDs::Callout::fill, s->getDialogFillColour().darker(0.025f));
    this->setColour(ColourIDs::Callout::frame, Colours::black.withAlpha(0.5f));

    // Rolls
    this->setColour(ColourIDs::Roll::blackKey, s->getBlackKeyColour().withMultipliedBrightness(0.95f));
    this->setColour(ColourIDs::Roll::blackKeyAlt, s->getBlackKeyColour());
    this->setColour(ColourIDs::Roll::whiteKey, s->getWhiteKeyColour());
    this->setColour(ColourIDs::Roll::whiteKeyAlt, s->getWhiteKeyColour().withMultipliedBrightness(1.05f));
    this->setColour(ColourIDs::Roll::rowLine, s->getRowColour());
    this->setColour(ColourIDs::Roll::barLine, s->getBarColour().withAlpha(0.8f));
    this->setColour(ColourIDs::Roll::barLineBevel, Colours::white.withAlpha(0.015f));
    this->setColour(ColourIDs::Roll::beatLine, s->getBarColour().withAlpha(0.4f));
    this->setColour(ColourIDs::Roll::snapLine, s->getBarColour().withAlpha(0.1f));

    const auto headerFill = s->getTimelineColour().darker(0.025f);
    this->setColour(ColourIDs::Roll::headerFill, headerFill);
    this->setColour(ColourIDs::Roll::headerBorder, Colours::white.withAlpha(0.045f));
    this->setColour(ColourIDs::Roll::headerSnaps, headerFill.contrasting().interpolatedWith(headerFill, 0.63f));
    this->setColour(ColourIDs::Roll::headerRecording, headerFill.interpolatedWith(Colours::red, 0.55f));

    this->setColour(ColourIDs::Roll::playheadShade, Colours::black.withAlpha(0.1f));
    this->setColour(ColourIDs::Roll::playheadPlayback, s->getLassoBorderColour().withAlpha(0.75f));
    this->setColour(ColourIDs::Roll::playheadRecording, s->getLassoBorderColour().interpolatedWith(Colours::red, 0.75f).withAlpha(0.55f));
    this->setColour(ColourIDs::Roll::trackHeaderFill, s->getWhiteKeyColour());
    this->setColour(ColourIDs::Roll::trackHeaderBorder, Colours::white.withAlpha(0.08f));

    this->setColour(ColourIDs::Roll::noteFill, textColour.interpolatedWith(Colours::white, 0.5f));
    this->setColour(ColourIDs::Roll::noteNameFill, s->getBlackKeyColour().darker(0.4f).withAlpha(0.95f));
    this->setColour(ColourIDs::Roll::noteNameBorder, textColour.withAlpha(0.4f));
    this->setColour(ColourIDs::Roll::noteNameShadow, textColour.withAlpha(0.25f));

    this->setColour(ColourIDs::Roll::draggingGuide, s->getLassoBorderColour().withAlpha(0.35f));
    this->setColour(ColourIDs::Roll::draggingGuideShadow, s->getLassoFillColour());

    this->setColour(ColourIDs::TransportControl::recordInactive, Colours::transparentBlack);
    this->setColour(ColourIDs::TransportControl::recordHighlight, Colours::red.withAlpha(0.35f));
    this->setColour(ColourIDs::TransportControl::recordActive, s->getTimelineColour().darker(0.05f).interpolatedWith(Colours::red, 0.5f));
    this->setColour(ColourIDs::TransportControl::playInactive, Colours::white.withAlpha(0.035f));
    this->setColour(ColourIDs::TransportControl::playHighlight, Colours::white.withAlpha(0.075f));
    this->setColour(ColourIDs::TransportControl::playActive, Colours::white.withAlpha(0.1f));

    this->setColour(ColourIDs::Logo::fill, textColour.withMultipliedAlpha(0.2f));
    this->setColour(ColourIDs::AudioMonitor::foreground, textColour);

    this->setColour(ColourIDs::VersionControl::connector, textColour.withAlpha(0.2f));
    this->setColour(ColourIDs::VersionControl::outline, textColour.withAlpha(0.3f));
    this->setColour(ColourIDs::VersionControl::highlight, textColour.withAlpha(0.02f));

    this->setColour(ColourIDs::RenderProgressBar::fill, s->getSidebarFillColour().darker(0.15f));
    this->setColour(ColourIDs::RenderProgressBar::outline, textColour.withAlpha(0.4f));
    this->setColour(ColourIDs::RenderProgressBar::progress, s->getSidebarFillColour().darker(1.f));
    this->setColour(ColourIDs::RenderProgressBar::waveform, textColour);

    this->setColour(ColourIDs::TapTempoControl::fill, textColour.withAlpha(0.015f));
    this->setColour(ColourIDs::TapTempoControl::fillHighlighted, textColour.withAlpha(0.05f));
    this->setColour(ColourIDs::TapTempoControl::outline, textColour.withAlpha(0.25f));

    // bright text probably means dark theme:
    this->isDarkTheme = textColour.getPerceivedBrightness() > 0.5f;

    // Update pre-rendered caches:
    constexpr int w = 128;
    constexpr int h = 128;

    {
        this->pageBackgroundA = Image(Image::ARGB, w, h, true);
        Graphics g(this->pageBackgroundA);
        g.setColour(this->findColour(ColourIDs::Backgrounds::pageFillA));
        g.fillAll();
        this->drawNoise(g, 0.5f);
    }

    {
        this->pageBackgroundB = Image(Image::ARGB, w, h, true);
        Graphics g(this->pageBackgroundB);
        g.setColour(this->findColour(ColourIDs::Backgrounds::pageFillB));
        g.fillAll();
        this->drawNoise(g, 0.5f);
    }

    {
        this->sidebarBackground = Image(Image::ARGB, w, h, true);
        Graphics g(this->sidebarBackground);
        g.setColour(this->findColour(ColourIDs::Backgrounds::sidebarFill));
        g.fillAll();
        this->drawNoise(g);
    }

    {
        this->headlineBackground = Image(Image::ARGB, w, h, true);
        Graphics g(this->headlineBackground);
        g.setColour(this->findColour(ColourIDs::Backgrounds::headlineFill));
        g.fillAll();
        this->drawNoise(g);
    }

    {
        this->dialogBackground = Image(Image::ARGB, w, h, true);
        Graphics g(this->dialogBackground);
        g.setColour(this->findColour(ColourIDs::Backgrounds::dialogFill));
        g.fillAll();
        this->drawNoise(g, 0.5f);
    }

    Icons::clearPrerenderedCache();
}
