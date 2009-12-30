#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <cstring>
#include <queue>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>

#include "geometry.h"
#include "options.h"
#include "sample.h"
#include "util.h"

#include "tiffio.h"

class Renderer;
class Grid;
class QuadGridSimple;

// Minimal wrapper around a renderer instance to provide control context for
// when surfaces push split/diced objects back into the render's queue.
class RenderQueueImpl : public RenderQueue
{
    private:
        Renderer& m_renderer;
        int m_splitDepth;
    public:
        RenderQueueImpl(Renderer& renderer, int splitDepth)
            : m_renderer(renderer),
            m_splitDepth(splitDepth)
        { }

        void push(const boost::shared_ptr<Geometry>& geom);
        void push(const boost::shared_ptr<Grid>& grid);
};


class Renderer
{
    private:
        // RenderQueueImpl is a friend so that it can appropriately push()
        // surfaces and grids into the renderer.
        friend class RenderQueueImpl;

        // Standard container for geometry metadata
        struct SurfaceHolder
        {
            boost::shared_ptr<Geometry> geom; //< Pointer to geometry
            int splitCount; //< Number of times the geometry has been split
            Box bound;      //< Bound in camera coordinates

            SurfaceHolder(const boost::shared_ptr<Geometry>& geom,
                          int splitCount, Box bound)
                : geom(geom),
                splitCount(splitCount),
                bound(bound)
            { }
        };

        // Ordering functor for surfaces in the render queue
        class surface_order
        {
            private:
                // desired bucket height in camera coordinates
                float m_bucketHeight;
            public:
                surface_order() : m_bucketHeight(16) {}

                bool operator()(const SurfaceHolder& a,
                                const SurfaceHolder& b) const
                {
                    float ya = a.bound.min.y;
                    float yb = b.bound.min.y;
                    if(ya < yb - m_bucketHeight)
                        return true;
                    else if(yb < ya - m_bucketHeight)
                        return false;
                    else
                        return a.bound.min.x < b.bound.min.x;
                }
        };

        typedef std::priority_queue<SurfaceHolder, std::vector<SurfaceHolder>,
                                    surface_order> SurfaceQueue;

        Options m_opts;
        SurfaceQueue m_surfaces;
        std::vector<Sample> m_samples;
        std::vector<float> m_image;
        Mat4 m_camToRas;

        void saveImage(const std::string& fileName);

        void initSamples();
        void push(const boost::shared_ptr<Geometry>& geom, int splitCount);
        void push(const boost::shared_ptr<Grid>& grid);

        template<typename GridT, typename PolySamplerT>
        void rasterize(GridT& grid);

        void rasterizeSimple(QuadGridSimple& grid);

    public:
        Renderer(const Options& opts, const Mat4& camToScreen = Mat4())
            : m_opts(opts),
            m_surfaces(),
            m_samples(),
            m_image(),
            m_camToRas()
        {
            // Set up camera -> raster matrix
            m_camToRas = camToScreen
                * Mat4().setScale(Vec3(0.5,-0.5,0))
                * Mat4().setTranslation(Vec3(0.5,0.5,0))
                * Mat4().setScale(Vec3(m_opts.xRes, m_opts.yRes, 1));
        }

        // Add geometry
        void add(const boost::shared_ptr<Geometry>& geom)
        {
            // TODO: Transform to camera space?
            push(geom, 0);
        }

        // Render all surfaces and save resulting image.
        void render();
};


#endif // RENDERER_H_INCLUDED
