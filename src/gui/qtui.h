#ifndef QTUI_HPP
#define QTUI_HPP

#include <QWidget>
#include <lvtk/ui.hpp>

namespace lvtk {

    template <bool Required = true>
    struct QtUI {

        template <class Derived>
        struct I : Extension<Required> {

            static void map_feature_handlers (FeatureHandlerMap& hmap) {
                /** Not required or implemented */
            }

            bool check_ok() {
                return true;
            }

            QWidget& container() {
                return _widget;
            }

        protected:

            LV2UI_Widget* widget() {
                return widget_cast (&_widget);
            }

        private:
            QWidget _widget;

        };
    };


} /* namespace lvtk */


#endif /* QTUI_HPP */
