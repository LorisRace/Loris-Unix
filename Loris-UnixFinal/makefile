.SILENT:

All:	Administrateur Client CreationBD BidonFichierPub Publicite Consultation Modification FichierUtilisateur.o Serveur

Administrateur:		mainAdmin.o windowadmin.o moc_windowadmin.o
					@echo "Administrateur compilé avec succès"
					g++  -o Administrateur mainAdmin.o windowadmin.o moc_windowadmin.o   /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -lpthread

mainAdmin.o:		mainAdmin.cpp
					@echo "mainAdmin.o compilé avec succès"
					g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o mainAdmin.o mainAdmin.cpp

windowadmin.o:		windowadmin.cpp
					@echo "windowadmin.o compilé avec succès"
					g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o windowadmin.o windowadmin.cpp

moc_windowadmin.o:	moc_windowadmin.cpp
					@echo "moc_windowadmin.o compilé avec succès"
					g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o moc_windowadmin.o moc_windowadmin.cpp					

Client:				dialogmodification.o mainClient.o windowclient.o moc_dialogmodification.o moc_windowclient.o
					@echo "Client compilé avec succès"
					g++  -o Client dialogmodification.o mainClient.o windowclient.o moc_dialogmodification.o moc_windowclient.o   /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -lpthread

dialogmodification.o:	dialogmodification.cpp
						@echo "dialogmodification.cpp compilé avec succès"
						g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o dialogmodification.o dialogmodification.cpp

mainClient.o:		mainClient.cpp
					@echo "mainClient.o compilé avec succès"
					g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o mainClient.o mainClient.cpp										

windowclient.o:		windowclient.cpp
					@echo "windowclient.o compilé avec succès"
					g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o windowclient.o windowclient.cpp


moc_dialogmodification.o:	moc_dialogmodification.cpp
							@echo "moc_dialogmodification.o compilé avec succès"
							g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o moc_dialogmodification.o moc_dialogmodification.cpp


moc_windowclient.o:			moc_windowclient.cpp
							@echo "moc_windowclient.o compilé avec succès"
							g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o moc_windowclient.o moc_windowclient.cpp

Serveur:			Serveur.cpp
					@echo "Serveur compilé avec succès"
					g++ Serveur.cpp -o Serveur FichierUtilisateur.o -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl


CreationBD:			CreationBD.cpp
					@echo "CreationBD compilée avec succès"
					g++ -o CreationBD CreationBD.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl


BidonFichierPub:	BidonFichierPub.cpp
					@echo "BidonFichierPub compilé avec succès"
					g++ -o BidonFichierPub BidonFichierPub.cpp


Publicite:			Publicite.cpp
					@echo "Publicite compilée avec succès"
					g++ -o Publicite Publicite.cpp


Consultation:		Consultation.cpp
					@echo "Consultation compilée avec succès"
					g++ Consultation.cpp -o Consultation -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl


Modification:		Modification.cpp
					@echo "Modification compilée avec succès"
					g++ Modification.cpp -o Modification -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

FichierUtilisateur.o:	FichierUtilisateur.cpp FichierUtilisateur.h
						@echo "FichierUtilisateur.o compilé avec succès"
						g++ FichierUtilisateur.cpp -c

clean:
		rm -f *.o

clobber:	clean
		rm -f Serveur Modification Consultation BidonFichierPub CreationBD Client Administrateur Publicite *.dat







