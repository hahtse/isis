/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#ifndef MAINWINDOW_CPP_
#define MAINWINDOW_CPP_

#include "MainWindowBase.hpp"
#include "ViewerCore.hpp"
#include "QGLWidgetImplementation.hpp"

namespace isis
{
namespace viewer
{

class MainWindow : public MainWindowBase
{


public:
	MainWindow( ViewerCore *core  );

private:
	struct Slot {
		MainWindow &parent;
		Slot( MainWindow &p ) : parent( p ) {}
	};
	ViewerCore *m_ViewerCore;



private:
	QGLWidgetImplementation *axialWidget;
};


}
} //end namespaces

#endif /* MAINWINDOW_CPP_ */
