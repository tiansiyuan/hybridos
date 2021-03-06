/*
** HFCL - HybridOS Foundation Class Library
**
** Copyright (C) 2018 Beijing FMSoft Technologies Co., Ltd.
**
** This file is part of HFCL.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef HFCL_VIEW_VIEW_H_
#define HFCL_VIEW_VIEW_H_

#include "../view/viewcontext.h"
#include "../common/event.h"
#include "../common/intrect.h"
#include "../common/realrect.h"

#include "../css/cssdeclared.h"
#include "../css/cssdeclaredgroup.h"
#include "../css/cssselector.h"
#include "../css/cssstackingcontext.h"
#include "../drawable/drawable.h"

#include <string>

namespace hfcl {

class GraphicsContext;
class ViewContainer;
class RootView;
class ScrollBar;
class CssComputed;
class CssBox;

enum {
    PAINT_STATUS_SHIFT = 24,
    PAINT_STATUS_MASK  = ((1 << PAINT_STATUS_SHIFT) - 1),
    PAINT_NO_HILIGHT = 0x80000000
};

typedef struct {
    View *view;
    int x;
    int y;
} ViewClickEventStruct;

enum {
    VIEW_TEXT_DIR_LTR,
    VIEW_TEXT_DIR_RTL,
    VIEW_TEXT_DIR_AUTO,
};

#define VIEW_COMMON_ATTR_ID         "id"
#define VIEW_COMMON_ATTR_DIR        "dir"
#define VIEW_COMMON_ATTR_ALT        "alt"
#define VIEW_COMMON_ATTR_NAME       "name"
#define VIEW_COMMON_ATTR_LANG       "lang"
#define VIEW_COMMON_ATTR_CLASS      "class"
#define VIEW_COMMON_ATTR_TITLE      "title"
#define VIEW_COMMON_ATTR_HIDDEN     "hidden"
#define VIEW_COMMON_ATTR_SUBJECT    "subject"
#define VIEW_COMMON_ATTR_TABINDEX   "tabindex"

class View : public Object {
public:
    View(const char* vtag, const char* vtype,
            const char* vclass, const char* vname, int vid);
    virtual ~View();

    bool attach(ViewContainer* parent);
    bool detach();

    /* operators for view relationship */
    const View* getPrev() const { return m_prev; }
    View* getPrev() { return m_prev; }
    void setPrev(View* view) { m_prev = view; }

    const View* getNext() const { return m_next; }
    View* getNext() { return m_next; }
    const View* getNextVisible() const { return m_next; }

    void setNext(View* view) { m_next = view; }

    const ViewContainer* getParent() const { return m_parent; }
    ViewContainer* getParent() { return m_parent; }
    void setParent(ViewContainer* view) {
        if (m_parent && m_parent != view) {
            detach();
        }
        if (view) {
            attach(view);
        }
    }

    const RootView* getRoot() const;
    RootView* getRoot();
    const CssComputed* getCssComputed() const { return m_css_computed; }
    CssComputed* getCssComputed() { return m_css_computed; }

    // return the HVML tag, e.g., hvroot, hvtext, hvimg, hvli, and so on
    const char* tag() const { return m_tag; }
    // return the HVML type, e.g., button, text, date, time, and so on
    const char* type() const { return m_type; }

    // compare tow view's tag (namespace), and attributes
    bool hasSameTagAndAttributes(const View* view) const;

    // methods to get/set additional data of the view
    void setAddData(const void *addData) { m_addData = addData; }
    const void *getAddData() const { return m_addData; }

    // methods to get/set name of the view
    const char* getName() const {
        return getAttribute(VIEW_COMMON_ATTR_NAME);
    }
    bool setName(const char* name) {
        return setAttribute(VIEW_COMMON_ATTR_NAME, name);
    }

    // methods to get/set alternative of the view
    const char* getAlt() const {
        return getAttribute(VIEW_COMMON_ATTR_ALT);
    }
    bool setAlt(const char* alt) {
        return setAttribute(VIEW_COMMON_ATTR_ALT, alt);
    }

    // helpers to get/set language of the view
    const char* getLang() const {
        return getAttribute(VIEW_COMMON_ATTR_LANG);
    }
    bool setLang(const char* lang) {
        return setAttribute(VIEW_COMMON_ATTR_LANG, lang);
    }

    // methods to get/set title of the view
    const char* getTitle() const;
    bool setTitle(const char* title);
    bool setTitle(HTStrId strId);

    // methods to get/set integer identifier of the view
    int getId() const;
    bool setId(int id);

    // methods to get/set tab index of the view
    int getTabIndex() const;
    bool setTabIndex(int tabIndex);

    // methods to get/set text direction of the view
    int getDir() const;
    bool setDir(int dir);

    // methods to get/set hidden attribute of the view
    void setHidden(bool b) {
        if (b)
            hide();
        else
            show();
    }
    void setVisible(bool b) {
        if (b)
            show();
        else
            hide();
    }

    bool isVisible() { return m_flags & VA_VISIBLE; }
    bool isHidden() { return !(m_flags & VA_VISIBLE); }
    void show() {
        setVisible(true);
        setAttribute(VIEW_COMMON_ATTR_HIDDEN, NULL);
    }
    void hide() {
        setVisible(false);
        setAttribute(VIEW_COMMON_ATTR_HIDDEN, "hidden");
    }

    // methods to get/set content of the view
    int getContentType() const;
    const char* getTextContent() const;
    HTResId getResourceContent() const;
    size_t getTextContentLength() const;
    bool setTextContent(const char* content);
    bool setTextContent(HTStrId strId);
    bool setResContent(HTResId resId);

    // If content type is resource, the view is replaced
    bool isReplaced() const { return m_contentResId != 0; }

    // Operators for CSS class of the view
    const char* getClass() const { return m_cssCls.c_str(); }
    bool setClass(const char* cssClses);
    bool includeClass(const char* cssCls);
    bool excludeClass(const char* cssCls);

    /* Operators to get/set flags of the view */
    bool isDisabled() { return m_flags & VA_DISABLED; }
    void disable() {
        bool old = setFlag(true, VA_DISABLED);
        if (old)
            onDisabled();
    }

    bool isEnabled() { return !(m_flags & VA_DISABLED); }
    void enable() {
        bool old = setFlag(false, VA_DISABLED);
        if (!old)
            onEnabled();
    }

    bool isChecked() { return m_flags & VA_CHECKED; }
    void check() {
        bool old = setFlag(true, VA_CHECKED);
        if (!old)
            onChecked();
    }
    void uncheck() {
        bool old = setFlag(false, VA_CHECKED);
        if (old)
            onUnchecked();
    }

    bool isActive() { return m_flags & VA_ACTIVE; }
    void activate() {
        bool old = setFlag (true, VA_ACTIVE);
        if (!old) {
            onActive();
        }
    }
    void deactivate(bool update = true) {
        bool old = setFlag (false, VA_ACTIVE);
        if (old) {
            onInactive();
        }
    }

    bool isFrozen() { return m_flags & VA_FROZEN; }
    void freeze() {
        bool old = setFlag (true, VA_FROZEN);
        if (!old) {
            onFrozen();
        }
    }
    void unfreeze(bool update = true) {
        bool old = setFlag (false, VA_FROZEN);
        if (old) {
            onUnfrozen();
        }
        if (update) {
            updateView ();
        }
    }

    bool isFocused() { return m_flags & VA_FOCUSED; }
    void focus() {
        bool old = setFlag(true, VA_FOCUSED);
        if (!old)
            onGotFocus();
    }
    void unfocus() {
        bool old = setFlag(false, VA_FOCUSED);
        if (!old)
            onLostFocus();
    }

    bool isFocusable() { return isEnabled() && m_flags & VA_FOCUSABLE; }
    bool setFocusable(bool b) { return setFlag(b, VA_FOCUSABLE); }

    bool isFocusStoppable() {
        return isFocusable() && m_flags & VA_FOCUSSTOPPABLE;
    }
    bool setFocusStoppable(bool b) { return setFlag(b, VA_FOCUSSTOPPABLE); }

    // Operators for user-defined attributes of the view
    const char* getAttribute(const char* attrKey) const;
    bool setAttribute(const char* attrKey, const char* attrValue);

    // Operators to check various attributes
    bool checkClass(const char* cssCls) const;
    bool checkPseudoElement(const char* pseudoEle) const;
    bool checkPseudoClass(const char* pseudoCls) const;
    bool checkAttribute(const char* attrKey, const char* attrValue) const;
    bool checkAttribute(const char* attrPair) const;

    virtual bool isContainer() const { return false; }
    virtual bool isRoot() const { return false; }
    virtual bool getIntrinsicWidth(HTReal* v) const { return false; }
    virtual bool getIntrinsicHeight(HTReal* v) const { return false; }
    virtual bool getIntrinsicRatio(HTReal* v) const { return false; }
    virtual bool getShrinkToFitWhidth(HTReal* preferred,
            HTReal* minimum) const { return false; }

    enum {
        VN_GOTFOCUS,
        VN_LOSTFOCUS,
        VN_HOVERED,
        VN_LEFT,
        VN_ACTIVE,
        VN_INACTIVE,
        VN_CLICKED,
        VN_DBLCLICKED,
        VN_VIEW_MAX,
    };

    /* virtual functions for attribute changes */
    virtual void onContainingBlockChanged();
    virtual void onStyleChanged(const char* attrKey);
    virtual void onContentChanged(const char* attrKey);
    virtual void onShown();
    virtual void onHidden();

    virtual void onDisabled();
    virtual void onEnabled();
    virtual void onChecked();
    virtual void onUnchecked();
    virtual void onGotFocus();
    virtual void onLostFocus();
    virtual void onHovered();
    virtual void onLeft();
    virtual void onActive();
    virtual void onInactive();

    virtual void onFrozen();
    virtual void onUnfrozen();
    virtual void onClicked();
    virtual void onDoubleClicked();
    virtual void onIdle();

    // virtual functions for drawing the view
    virtual void drawBackground(GraphicsContext* context, IntRect &rc);
    virtual void drawContent(GraphicsContext* context, IntRect &rc);
    virtual void drawScrollBar(GraphicsContext* context, IntRect &rc);
    virtual void onPaint(GraphicsContext* context);

    /*
     * virtual callbacks to handle the interaction events
     * Return false if the view is not intersted in the event.
     */
    virtual bool onKeyEvent(const KeyEvent& evt) { return false; }
    virtual bool onMouseEvent(const MouseEvent& evt) { return false; }
    virtual bool onMouseWheelEvent(const MouseWheelEvent& evt) { return false; }
    virtual bool onChar(const char* mchar) { return false; }

    virtual void viewToWindow(int *x, int *y);
    virtual void windowToView(int *x, int *y);

    void addEventListener(EventListener* listener);
    void removeEventListener(EventListener* listener);

    // the style sheet specified on the fly
    CssDeclared* myCss() { return &m_cssd_user; }

    // Change style on the fly
    bool getProperty(CssPropertyIds pid, Uint32 *value,
            HTPVData* data);
    bool setProperty(CssPropertyIds pid, Uint32 value,
            HTPVData data);

    // x,y coordiat from view, e.g (0,0) is same as m_rect.left, m_rect.top
    void updateView(int x, int y, int w, int h);
    void updateView(const IntRect &rc);

    virtual void onChildUpdateView(View *child,
            int x, int y, int w, int h, bool upBackGnd = true);
    void updateViewRect(const IntRect &rc);
    void updateViewRect();
    void updateView(bool upBackGnd = true);

    /* to be deprecated */
    virtual void focusMe();
    bool setFocus(View *view);

    virtual bool isWrapperView() { return false; }

    void getSize(int *w, int *h) {
        if (w) *w = m_rect.width();
        if (h) *h = m_rect.height();
    }

    const IntRect& getRect() const { return m_rect; }
    const IntRect& getViewportRect() const { return m_rc_viewport; }

    void setPosition(int x, int y, bool update) {
        IntRect rc(x, y, m_rect.width() + x, m_rect.height() + y);

        if (update)
            setRect(rc);
        else
            m_rect = rc;
    }

    void getPosition(int *x, int *y) {
        if (x) *x = m_rect.left();
        if (y) *y = m_rect.top();
    }

    View(View* parent, DrawableSet* drset);
    View(int id, int x, int y, int w, int h);

    bool setRect(int left, int top, int right, int bottom) {
        return setRect(IntRect(left, top, right, bottom));
    }

    bool setRectWH(int left, int top, int width, int height) {
        return setRect(IntRect(left, top, width + left, height + top));
    }

    bool setRectNoUpdate(int left, int top, int right, int bottom) {
        IntRect irc = IntRect(left, top, right, bottom);
        if (irc == m_rect)
            return false;

        if (m_rect.width() == 0 || m_rect.height() == 0) {
            // VincentWei: for the initial set, do not call updateView()
            m_rect = irc;
            return false;
        }
        else if (m_rect != irc) {
            // VincentWei: call updateView only set a different rect
            m_rect = irc;
            return true;
        }
        return false;
    }

    virtual bool setRect(const IntRect& irc) {
        if (irc == m_rect)
            return false;

        if (m_rect.width() == 0 || m_rect.height() == 0) {
            // VincentWei: for the initial set, do not call updateView()
            m_rect = irc;
        }
        else if (m_rect != irc) {
            // VincentWei: call updateView only set a different rect
            m_rect = irc;
            updateView ();
        }
        return true;
    }

    friend class ViewContainer;
    friend class RootView;

protected:
    enum {
        VA_DISABLED         = (0x01 << 0),
        VA_HOVER            = (0X01 << 1),
        VA_ACTIVE           = (0x01 << 2),
        VA_CHECKED          = (0x01 << 3),
        VA_FOCUSED          = (0x01 << 4),
        VA_FOCUSABLE        = (0x01 << 5),
        VA_FOCUSSTOPPABLE   = (0x01 << 6),
        VA_FROZEN           = (0x01 << 7),
        VA_VISIBLE          = (0x01 << 8),
        FLAG_SHIFT          = 9
    };

    inline bool setFlag(bool b, unsigned int flag) {
        bool old = m_flags & flag;
        if(b)
            m_flags |= flag;
        else
            m_flags &= (~flag);
        return old;
    }

    char* m_tag;
    char* m_type;

    std::string m_cssCls;

    HTStrId m_contentStrId;
    HTResId m_contentResId;
    std::string m_contentStr;

    HTStrId m_titleStrId;
    std::string m_titleStr;

    const void* m_addData;
    Uint32 m_flags;

    ViewContainer *m_parent;
    View *m_prev;
    View *m_next;

    MAPCLASSKEY(utf8string, utf8string, AttributesMap);
    AttributesMap m_attrs;

    /* CSS-related members */
    // user-defined CSS (properties specified on the fly)
    CssDeclared m_cssd_user;
    // All selected CssDeclared objects for the view
    CssDeclaredGroupWithSpecificity m_cssdg_static;
    CssDeclaredGroupWithSpecificity m_cssdg_dynamic;
    // The computed values of all CSS properties.
    CssComputed* m_css_computed;

    /* virtual functions for rendering */
    // compute CSS property values
    virtual void applyCss(CssDeclared* css, const CssSelectorGroup& selector);
    virtual void computeCss();

    virtual void makeCssBox();
    virtual void makeStackingContext(CssStackingContext* cssSc);
    virtual void layOut(CssBox* ctnBlock);

    // The pricipal box of the view, either a CssBox,
    // a CssLineBoxContainer, or a CssBlockBoxContainer
    CssBox* m_cssbox_principal;
    // The additional box of the view if Property display is list-item.
    CssBox* m_cssbox_additional;

    IntRect m_rc_viewport;
    IntRect m_rect;

    LISTEX(EventListener *, EventListenerList,
            do { return *v1 == *v2; } while (0),
            do { (*n)->unref(); } while (0));
    EventListenerList m_listeners;
    void releaseEventListeners();

    bool raiseViewEvent(ViewEvent *event);

    /* Members related to rendering */
    virtual void inner_updateView(int x, int y, int w, int h,
            bool upBackGnd = true);
    void inner_updateViewRect(int x, int y, int w, int h);

private:

    /* Members for scroll bars */
    ScrollBar* m_vsb;
    ScrollBar* m_hsb;
};

} // namespace hfcl

#endif /* HFCL_VIEW_VIEW_H_ */
