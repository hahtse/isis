#ifndef ISISPROPERTYVIEWER_H
#define ISISPROPERTYVIEWER_H

#include "ui_isisPropertyViewer.h"

#include "DataStorage/io_factory.hpp"
#include "CoreUtils/log.hpp"
#include "boost/shared_ptr.hpp"


class isisPropertyViewer : public QMainWindow
{
	Q_OBJECT

public:
    isisPropertyViewer(QMainWindow *parent = 0);
	 
private slots:
	void on_action_Close_activated();
	void on_action_Open_activated();
		
private:
     Ui::isisPropertyViewer ui;
	 void addToTree(const boost::shared_ptr<isis::data::Image> image) const;
	 
	
	 
};
 
#endif