/*
* File:			main.cxx
* Description:	A utility to build a graph from tirangled surface, and to extract connected points.
* Author:		Zhang Teng, PhD Candidate
* Organization:	Department of Imaging and Interventional Radiology, Chinese University of Hong Kong
* Mailbox:		zhangteng630@gmail.com
* License:		GNU GPL
*
* Created on January 15, 2016, 10:21 AM
*/

#include <algorithm>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkLine.h>
#include <vtkSphereSource.h>
#include <vtkTriangleFilter.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkDataSetMapper.h>

int main(int argc, char * argv[])
{
	//create a sphere mesh surface
	vtkSmartPointer< vtkSphereSource > sphereSource = vtkSmartPointer< vtkSphereSource >::New();
	sphereSource->SetRadius(5.0);
	sphereSource->Update();
	vtkSmartPointer< vtkTriangleFilter > triangleFilter = vtkSmartPointer< vtkTriangleFilter >::New();
	triangleFilter->SetInputData(sphereSource->GetOutput());
	triangleFilter->Update();
	vtkSmartPointer< vtkPolyData > surface = triangleFilter->GetOutput();

	//find connected points
	std::vector< std::vector< vtkIdType > >  connectedPoints;
	connectedPoints.resize(surface->GetNumberOfPoints());
	for(vtkIdType cellId = 0; cellId < surface->GetNumberOfCells(); ++cellId)
	{
		vtkCell * cell = surface->GetCell(cellId);
		for(vtkIdType cellPointId0 = 0; cellPointId0 < cell->GetNumberOfPoints(); ++cellPointId0)
		{
			vtkIdType pointId0 = cell->GetPointId(cellPointId0);
			for(vtkIdType cellPointId1 = 0; cellPointId1 < cell->GetNumberOfPoints(); ++cellPointId1)
			{
				if(cellPointId0 == cellPointId1)
					continue;
				vtkIdType pointId1 = cell->GetPointId(cellPointId1);
				connectedPoints[pointId0].push_back(pointId1);
			}
		}
	}
	//unique
	for(size_t i = 0; i < connectedPoints.size(); ++i)
	{
		std::sort(connectedPoints[i].begin(), connectedPoints[i].end());
		connectedPoints[i].erase(std::unique(connectedPoints[i].begin(), connectedPoints[i].end()), connectedPoints[i].end());
	}

	//rebuild a surface with lines as cells
	vtkSmartPointer< vtkPolyData > mesh = vtkSmartPointer< vtkPolyData >::New();
	mesh->SetPoints(surface->GetPoints());
	vtkSmartPointer< vtkCellArray > lines = vtkSmartPointer< vtkCellArray >::New();
	for(size_t i = 0; i < connectedPoints.size(); ++i)
	{
		vtkIdType pid0 = static_cast<vtkIdType>(i);
		for(size_t j = 0; j < connectedPoints[i].size(); ++j)
		{
			vtkIdType pid1 = static_cast<vtkIdType>(connectedPoints[i][j]);
			if(pid1 <= pid0)
				continue;
			vtkSmartPointer< vtkLine > line = vtkSmartPointer< vtkLine >::New();
			line->GetPointIds()->SetId(0, pid0);
			line->GetPointIds()->SetId(1, pid1);
			lines->InsertNextCell(line);
		}
	}
	mesh->SetLines(lines);

	vtkSmartPointer< vtkDataSetMapper > meshMapper = vtkSmartPointer< vtkDataSetMapper >::New();
	meshMapper->SetInputData(mesh);	
	vtkSmartPointer< vtkActor > meshActor = vtkSmartPointer< vtkActor >::New();
	meshActor->SetMapper(meshMapper);
	meshActor->GetProperty()->SetPointSize(3);
	meshActor->GetProperty()->SetColor(128, 128, 128);
	
	//node
	int pid = 0;
	vtkSmartPointer< vtkPoints > nodePoint = vtkSmartPointer< vtkPoints >::New();
	nodePoint->InsertNextPoint(surface->GetPoint(pid));
	vtkSmartPointer< vtkPolyData > nodePoly = vtkSmartPointer< vtkPolyData >::New();
	nodePoly->SetPoints(nodePoint);
	vtkSmartPointer<vtkVertexGlyphFilter> nodeVertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	nodeVertexFilter->SetInputData(nodePoly);
	nodeVertexFilter->Update();
	vtkSmartPointer< vtkDataSetMapper > nodeMapper = vtkSmartPointer< vtkDataSetMapper >::New();
	nodeMapper->SetInputData(nodeVertexFilter->GetOutput());
	vtkSmartPointer< vtkActor > nodeActor = vtkSmartPointer< vtkActor >::New();
	nodeActor->SetMapper(nodeMapper);
	nodeActor->GetProperty()->SetColor(255,0,0);
	nodeActor->GetProperty()->SetPointSize(5);
	
	//neighbors	
	vtkSmartPointer< vtkPoints > neighborPoint = vtkSmartPointer< vtkPoints >::New();
	for(size_t i = 0; i < connectedPoints[pid].size(); ++i)
	{
		neighborPoint->InsertNextPoint(surface->GetPoint(connectedPoints[pid][i]));
	}
	vtkSmartPointer< vtkPolyData > neighborPoly = vtkSmartPointer< vtkPolyData >::New();
	neighborPoly->SetPoints(neighborPoint);
	vtkSmartPointer<vtkVertexGlyphFilter> neighborVertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	neighborVertexFilter->SetInputData(neighborPoly);
	neighborVertexFilter->Update();
	vtkSmartPointer< vtkDataSetMapper > neighborMapper = vtkSmartPointer< vtkDataSetMapper >::New();
	neighborMapper->SetInputData(neighborVertexFilter->GetOutput());
	vtkSmartPointer< vtkActor > neighborActor = vtkSmartPointer< vtkActor >::New();
	neighborActor->SetMapper(neighborMapper);
	neighborActor->GetProperty()->SetColor(0,255,0);
	neighborActor->GetProperty()->SetPointSize(5);
	
	vtkSmartPointer< vtkRenderer > renderer = vtkSmartPointer< vtkRenderer >::New();
	renderer->AddActor(meshActor);
	renderer->AddActor(nodeActor);
	renderer->AddActor(neighborActor);
	
	vtkSmartPointer< vtkRenderWindow > renderWindow = vtkSmartPointer< vtkRenderWindow >::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer< vtkRenderWindowInteractor > interactor = vtkSmartPointer< vtkRenderWindowInteractor >::New();
	interactor->SetRenderWindow(renderWindow);
	
	interactor->Initialize();
	interactor->Start();
	renderWindow->Render();
	
	return 0;
}
