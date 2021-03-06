/* This file is part of ArpmanetDC. Copyright (C) 2012
 * Source code can be found at http://code.google.com/p/arpmanetdc/
 * 
 * ArpmanetDC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ArpmanetDC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ArpmanetDC.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef CONTAINERTHREAD_H
#define CONTAINERTHREAD_H

#include <QObject>
#include <QHash>
#include <QPair>
#include <QFileInfo>
#include <QStringList>
#include <QHostAddress>
#include "util.h"

#define CONTAINER_EXTENSION "adcc"

#define CONTAINER_PREAMBLE "..::ArpmanetDCContainer::.."
#define CONTAINER_PREAMBLE_SIZE 27

#define HEADER_LENGTH 16+27

typedef QPair<quint64, QHash<QString, quint64> > ContainerContentsType;

struct ContainerLookupReturnStruct
{
    QString filePath;
    QByteArray rootTTH;
    quint64 fileSize;
};

class ContainerThread : public QObject
{
    Q_OBJECT
public:
    ContainerThread(QObject *parent = 0);
    ~ContainerThread();

public slots:
    //Request all containers in a directory
    void requestContainers(QString containerDirectory);
    //Save the containers to files in the directory specified
    void saveContainers(QHash<QString, ContainerContentsType> containerHash, QString containerDirectory);
    //Return hashes from DB
    void returnTTHsFromPaths(QHash<QString, QList<ContainerLookupReturnStruct> > results, QString containerPath);

    //Process downloaded container
    void processContainer(QHostAddress host, QString containerPath, QString downloadPath);

signals:
    //Return the containers requested
    void returnContainers(QHash<QString, ContainerContentsType> containerHash);
    //Request hashes from a list of filepaths from DB
    void requestTTHsFromPaths(QHash<QString, QStringList> filePaths, QString containerPath);

    //Return processed container information
    void returnProcessedContainer(QHostAddress host, ContainerContentsType index, QList<ContainerLookupReturnStruct> data, QString downloadPath, QString containerName);

    //Containers saved
    void containersSaved();

private:
    //Process a file
    ContainerContentsType processContainerFileIndex(QFileInfo fileInfo);
    //Write a file
    bool writeContainerFileIndex(QString name, ContainerContentsType contents);
    //Clean container directory of all containers
    bool cleanContainers(QString containerDirectory);

    int pContainerCount;
};

#endif