/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License

About this File
Constructing Direct2d paths

Notes

For Future Consideration
*/


#ifndef CORONA_PATHS_H
#define CORONA_PATHS_H


namespace corona {


	class path {
	public:
		ID2D1PathGeometry* geometry;
		ID2D1GeometrySink* sink;

		path(direct2dContextBase* ptarget) : geometry(NULL), sink(NULL)
		{
			HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

			if (ptarget)
			{
				if (ptarget->getDeviceContext())
				{
					if (auto pfactory = ptarget->getAdapter().lock()) {
						hr = pfactory->getD2DFactory()->CreatePathGeometry(&geometry);
						if (not SUCCEEDED(hr)) {
							// UH, SOMETHING;
						}
					}
				}
			}
		}

		~path()
		{
			if (sink) sink->Release();
			if (geometry) geometry->Release();
			sink = NULL;
			geometry = NULL;
		}

		void start_figure(D2D1_POINT_2F point)
		{
			if (geometry) {
				geometry->Open(&sink);
				if (sink)
					sink->BeginFigure(point, D2D1_FIGURE_BEGIN_FILLED);
			}
		}

		void add_line(D2D1_POINT_2F point)
		{
			if (sink) sink->AddLine(point);
		}

		void add_arc(D2D1_POINT_2F point1, D2D1_SIZE_F size1, FLOAT rotationAngle)
		{
			D2D1_SWEEP_DIRECTION direction = rotationAngle > 0.0 ? D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE : D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_CLOCKWISE;
			rotationAngle = fabs(rotationAngle);
			D2D1_ARC_SIZE arcSize = rotationAngle > 180.0 ? D2D1_ARC_SIZE::D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE::D2D1_ARC_SIZE_SMALL;
			if (sink) sink->AddArc(D2D1::ArcSegment(point1, size1, fabs(rotationAngle), direction, arcSize));
		}

		void add_bezier(D2D1_POINT_2F point1, D2D1_POINT_2F point2, D2D1_POINT_2F point3)
		{
			if (sink) sink->AddBezier(D2D1::BezierSegment(point1, point2, point3));
		}

		void add_quadratic_bezier(D2D1_POINT_2F point1, D2D1_POINT_2F point2)
		{
			if (sink) sink->AddQuadraticBezier(D2D1::QuadraticBezierSegment(point1, point2));
		}

		void close_figure(bool closed)
		{
			if (sink) {
				sink->EndFigure(closed ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
				sink->Close();
				sink->Release();
				sink = NULL;
			}
		}
	};

}

#endif
