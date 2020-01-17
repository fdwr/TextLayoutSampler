#if 0

// Call from DrawableObjectAndValues::Draw()

DrawableObjectAndValues::Draw()
{
    ...
    DrawingCanvas::RawPixels rawPixels = drawingCanvas.GetRawPixels();
    std::vector<Edge> edges = { {0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,0},  {8,9},{9,10},{10,11},{11,8},  {12,13},{13,14},{14,15},{15,12} };
    std::vector<PointI> points = { {100,100},{150,0},{250,150},{250,0},{300,100},{250,200},{150,50},{150,200},  {400,150},{600,200},{350,350},{300,250},  {400,200},{450,250},{400,275},{350,250} };
    FillPolyline(rawPixels, canvasTransform, points, IN OUT edges, 0xFFE080C0);
    DrawLineTransformed(
        rawPixels,
        canvasTransform,
        100,
        100,
        100,
        200,
        0xFFE080C0
    );
}


namespace
{
    struct PointI // There is no D2D_POINT_2I, just D2D_POINT_2F, D2D_POINT_2F, D2D_POINT_2L.
    {
        int32_t x, y;
    };

    struct Intersection
    {
        int32_t x; // Current x coordinate.
        int32_t dx; // Delta x per y, where x += dx each row.
        uint32_t modulusAccumulator; // Current accumulator. If it exceeds limit, wrap and add +1 to x.
        uint32_t modulusIncrement; // Amount to increase the accumulator each 
        uint32_t modulusLimit; // Upper limit, which actually just equals the line's height.
        int32_t yEnd; // Ending y coordinate where this intersection stops.
    };

    bool operator < (Intersection const& intersection1, Intersection const& intersection2) noexcept
    {
        return intersection1.x < intersection2.x;
    }

    using Edge = std::pair<uint32_t,uint32_t>;

    void OrderEdgesDownThenRight(array_ref<PointI const> points, _Inout_ array_ref<Edge> edges)
    {
        // Ensure all edges are ordered pointing downward, with the top y coordinate first.
        for (auto& edge : edges)
        {
            assert(edge.first < points.size());
            assert(edge.second < points.size());
            auto const& pt1 = points[edge.first];
            auto const& pt2 = points[edge.second];
            if (pt1.y > pt2.y)
            {
                std::swap(edge.first, edge.second);
            }
        }

        // Order points top to bottom, then left to right for any points on the
        // same row. Order is based on the first point, ignoring the second.
        std::sort(
            edges.begin(),
            edges.end(),
            [&, points](Edge const& edge1, Edge const& edge2)
        {
            auto const& pt1 = points[edge1.first];
            auto const& pt2 = points[edge2.first];
            if (pt1.y < pt2.y) return true;
            if (pt1.y > pt2.y) return false;
            if (pt1.x < pt2.x) return true;
            if (pt1.x > pt2.x) return false;
            return edge1.first < edge2.first;
        }
        );
    }

    void DrawEdgeListSingleRow(
        array_ref<uint32_t> pixelRow,
        array_ref<Intersection const> intersectionList,
        uint32_t color
    )
    {
        // Draw a single horizontal row of alternating spans between intersection point pairs.
        int32_t parity = 0;
        int32_t previousX = INT_MAX;
        int32_t maxX = int32_t(pixelRow.size());
        for (auto& intersection : intersectionList)
        {
            auto nextX = intersection.x;
            if (previousX < maxX)
            {
                // Color the x-span pixels while the crossing parity is odd.
                if (parity && nextX > 0)
                {
                    auto pixelSpan = pixelRow.clamped_slice(previousX, nextX);
                    for (uint32_t& pixel : pixelSpan)
                    {
                        pixel = color;
                    }
                }
            }
            parity ^= 1;
            previousX = nextX;
        }
    }

    void InitializeIntersectionFromEdge(
        array_ref<PointI const> points,
        Edge edge,
        int32_t y,
        _Out_ Intersection& intersection
    )
    {
        // Set initial x and final y.
        auto const& pt1 = points[edge.first];
        auto const& pt2 = points[edge.second];
        intersection.x = pt1.x;
        intersection.yEnd = pt2.y;
        intersection.modulusAccumulator = 0;

        // Determine the fractional x delta each row.
        auto dy = pt2.y - pt1.y;
        auto dx = pt2.x - pt1.x;
        if (dy > 0)
        {
            intersection.dx = dx / dy;
            intersection.modulusIncrement = abs(dx) % dy;
            intersection.modulusLimit = dy;
            // Handle negative dx values by reversing the increment fraction.
            // That way advancing each row requires no special logic for negative.
            if (dx < 0)
            {
                --intersection.dx;
                intersection.modulusIncrement = intersection.modulusLimit - intersection.modulusIncrement;
            }

            // For the case where a line is partly clipped and starts above screen,
            // advance the intersection point. It would be equivalent but slower to
            // loop calling AdvanceIntersectionsXCoordinateOneRowDown.
            if (y > pt1.y)
            {
                auto leadingClippedHeight = y - pt1.y;
                intersection.x += intersection.dx * leadingClippedHeight;
                uint64_t extendedAccumulator = uint64_t(intersection.modulusIncrement) * leadingClippedHeight;
                intersection.x += uint32_t(extendedAccumulator / intersection.modulusLimit);
                intersection.modulusAccumulator += uint32_t(extendedAccumulator % intersection.modulusLimit);
            }
        }
        else // Just use zeroes for a zero width line or perfectly horizontal line.
        {
            intersection.dx = 0;
            intersection.modulusIncrement = 0;
            intersection.modulusLimit = INT_MAX;
        }
    }

    void AdvanceIntersectionsXCoordinateOneRowDown(_Inout_ array_ref<Intersection> intersectionList)
    {
        // Nudge the edge intersection's current x pixel by the delta per single y pixel.
        for (auto& intersection : intersectionList)
        {
            intersection.x += intersection.dx;

            // If the accumulator overflows, move an extra pixel.
            intersection.modulusAccumulator += intersection.modulusIncrement;
            if (intersection.modulusAccumulator >= intersection.modulusLimit)
            {
                intersection.modulusAccumulator -= intersection.modulusLimit; // modulus operation.
                ++intersection.x;
            }
        }
    }

    void OrderIntersectionListByXCoordinate(_Inout_ array_ref<Intersection> intersectionList)
    {
        // Check whether it's still in order, reordering if stale.
        if (!std::is_sorted(intersectionList.begin(), intersectionList.end()))
        {
            std::sort(intersectionList.begin(), intersectionList.end());
        }
    }

    void FillPolyline(
        DrawingCanvas::RawPixels rawPixels,
        DX_MATRIX_3X2F const& canvasTransform,
        array_ref<PointI> points,
        _Inout_ array_ref<Edge> edges,
        uint32_t fillColor
    );

    void DrawLineTransformed(
        DrawingCanvas::RawPixels rawPixels,
        DX_MATRIX_3X2F const& canvasTransform,
        int32_t x1,
        int32_t y1,
        int32_t x2,
        int32_t y2,
        uint32_t color
    ) throw()
    {
        // Transform points first.
        PointI ends[2] = {{x1,y1}, {x2,y2}};

        for (auto& point : ends)
        {
            auto x = int32_t(point.x * canvasTransform.xx + point.y * canvasTransform.yx + canvasTransform.dx);
            auto y = int32_t(point.x * canvasTransform.xy + point.y * canvasTransform.yy + canvasTransform.dy);
            point.x = x;
            point.y = y;
        }

        PointI points[4] = {ends[0],ends[0],ends[1],ends[1]};
        auto dx = ends[1].x - ends[0].x;
        auto dy = ends[1].y - ends[0].y;
        if (abs(dx) >= abs(dy)) // More horizontal than vertical.
        {
            points[0].y += 1;
            points[3].y += 1;
        }
        else // More vertical than horizontal.
        {
            points[1].x += 1;
            points[2].x += 1;
        }

        Edge edges[4] = {{0,1},{1,2},{2,3},{3,0}};
        FillPolyline(rawPixels, canvasTransform, points, IN OUT edges, color);
    }

    void FillPolyline(
        DrawingCanvas::RawPixels rawPixels,
        DX_MATRIX_3X2F const& canvasTransform,
        array_ref<PointI> points,
        _Inout_ array_ref<Edge> edges,
        uint32_t fillColor
    )
    {
        assert(rawPixels.bitsPerPixel == 32);

        if (edges.empty())
            return;

        std::vector<Intersection> intersections;
        intersections.reserve(edges.size());

        // Rhombus
        //std::vector<Edge> edges = {{0,1},{1,2},{2,3},{3,0}};
        //std::vector<PointI> points = {{100,100},{300,150},{50,300},{0,200}};

        // Diamond
        //std::vector<Edge> edges = {{0,1},{1,2},{2,3},{3,0}};
        //std::vector<PointI> points = {{100,100},{300,200},{100,300},{0,200}};

        // Mountains
        //std::vector<Edge> edges = {{0,1},{1,2},{2,3},{3,4},{4,0}};
        //std::vector<PointI> points = {{100,100},{150,0},{200,50},{250,0},{300,100}};

        // Two mountains
        //std::vector<Edge> edges = {{0,1},{1,2},{2,3},{3,4},{4,0}};
        //std::vector<PointI> points = {{100,100},{150,0},{200,100},{250,0},{300,100}};

        // Crossing
        //std::vector<Edge> edges = {{0,1},{1,2},{2,3},{3,4},{4,0}};
        //std::vector<PointI> points = {{100,100},{150,0},{200,200},{250,0},{300,100}};

        // Coincident edge test.
        //std::vector<Edge> edges = {{0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,0}};
        //std::vector<PointI> points = {{100,100},{150,0},{250,150},{250,0},{300,100},{250,200},{150,0}};

        // Inset shape.
        //std::vector<Edge> edges = {{0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,0}};
        //std::vector<PointI> points = {{100,100},{150,0},{250,150},{250,0},{300,100},{250,200},{150,50},{150,200}};

        // Inset shape plus rhombus.
        //std::vector<Edge> edges = {{0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,0},  {8,9},{9,10},{10,11},{11,8},  {12,13},{13,14},{14,15},{15,12}};
        //std::vector<PointI> points = {{100,100},{150,0},{250,150},{250,0},{300,100},{250,200},{150,50},{150,200},  {400,150},{600,200},{350,350},{300,250},  {400,200},{450,250},{400,275},{350,250}};

        // Single pixel thick sheared line.
        //std::vector<Edge> edges = {{0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {8,9},{9,10},{10,11},{11,8}};
        //std::vector<PointI> points = {{100,100},{101,100},{201,300},{200,300},  {150,100},{151,100},{251,200},{250,200}, {200,101},{200,100},{400,200},{400,201}};

#if 1
        static float a = 0, b = 0;
        int i = 0;
        for (auto& point : points)
        {
            auto c = cos(b), s = sin(b);
            //auto x = int32_t((point.x - 200) *  c + (point.y - 200) * s) + 200;
            //auto y = int32_t((point.x - 200) * -s + (point.y - 200) * c) + 200;
            //point.x = x;
            //point.y = y;
            point.x = int32_t((point.x - 200) *  c + (point.y - 200) * s) + 300;
            point.y = int32_t((point.x - 200) * -s + (point.y - 200) * c);// +300;
                                                                          //point.x += int32_t(cos(a + i) * 40.0f);
                                                                          //point.y += int32_t(sin(a + i) * 40.0f);
            ++i;
        }
        a += 0.1f;
        b -= 0.01f;
#endif
#if 1
        for (auto& point : points)
        {
            auto x = int32_t(point.x * canvasTransform.xx + point.y * canvasTransform.yx + canvasTransform.dx);
            auto y = int32_t(point.x * canvasTransform.xy + point.y * canvasTransform.yy + canvasTransform.dy);
            point.x = x;
            point.y = y;
        }
#endif

        OrderEdgesDownThenRight(points, IN OUT edges);

        auto pixelRow = array_ref<uint32_t>(reinterpret_cast<uint32_t*>(rawPixels.pixels), rawPixels.width);

        // Find top and bottom to reduce number of scanlines.
        // Then clamp to drawing surface.
        int32_t yStart = INT_MAX;
        int32_t yEnd   = -INT_MAX;
        for (auto const& point : points)
        {
            yStart = std::min(yStart, point.y);
            yEnd   = std::max(yEnd, point.y);
        }
        yStart = std::max(yStart, 0);
        yEnd   = std::min(yEnd, int32_t(rawPixels.height));
        pixelRow.offset_by_bytes(rawPixels.byteStride * yStart);

        auto currentEdge = edges.begin();
        int32_t nextSignificantY = 0;

        for (int32_t y = yStart; y < yEnd; ++y)
        {
            // Find any new line segments that start at this row. Otherwise skip over several rows
            // until reaching another edge rather than check every time through the loop.
            if (y >= nextSignificantY)
            {
                nextSignificantY = yEnd;
                for (; currentEdge != edges.end(); ++currentEdge)
                {
                    auto& edge = *currentEdge;
                    auto edgeStartingY = points[edge.first].y;
                    if (edgeStartingY <= y && points[edge.second].y > y)
                    {
                        // Add new intersection point to the active list, initializing the slope and other vars.
                        intersections.resize(intersections.size() + 1);
                        InitializeIntersectionFromEdge(points, edge, y, OUT intersections.back());
                    }
                    else if (edgeStartingY > y)
                    {
                        // This edge begins beyond the current row (as do all edges after it, since they are sorted).
                        // So just store this row as significant for future row loops.
                        nextSignificantY = std::min(nextSignificantY, edgeStartingY);
                        break;
                    }
                }

                // Delete old intersections upon reaching their last y coordinate.
                auto intersectionsEnd = std::remove_if(intersections.begin(), intersections.end(), [y](Intersection const& intersection) {return intersection.yEnd == y; });
                intersections.erase(intersectionsEnd, intersections.end());
                std::for_each(intersections.begin(), intersections.end(), [&](auto const& intersection) {nextSignificantY = std::min(nextSignificantY, intersection.yEnd); });
            }

            // Ensure the intersections are still in ascending x order after adding any new ones or deleting old ones.
            // Even if none were added, just advancing the intersection lines can cross each other.
            OrderIntersectionListByXCoordinate(IN OUT intersections);

            DrawEdgeListSingleRow(pixelRow, intersections, fillColor);

            AdvanceIntersectionsXCoordinateOneRowDown(IN OUT intersections);

            pixelRow.offset_by_bytes(rawPixels.byteStride);
        }
    }
}
#endif
