#ifndef __HFST_LINEAR_TEST_TYPE_H__
#define __HFST_LINEAR_TEST_TYPE_H__

#include "pch.h"

namespace HFST
{
    static void SpiralTestHelper( int theta, double* xPos, double* yPos, size_t nRange );

    class ILinearTest
    {
    public:
        explicit ILinearTest(float nLaps) :m_Laps(nLaps), m_PreX(0.0), m_PreY(0.0) {};
        virtual ~ILinearTest() {};

        virtual void Run( void* pDlg ) = 0;
    protected:
        inline void SetRunStatus(void* pDlg, int theta);
        inline void SetFinishStatus(void* pDlg);
    protected:
        float   m_Laps;
        double  m_PreX;
        double  m_PreY;
    };

    class Spiral_Clockwise_InOut : public ILinearTest
    {
    public:
        explicit Spiral_Clockwise_InOut(float nLaps );
        ~Spiral_Clockwise_InOut();

        void Run(void* pDlg) override;
    };

    class Spiral_Clockwise_OutIn : public ILinearTest
    {
    public:
        explicit Spiral_Clockwise_OutIn(float nLaps);
        ~Spiral_Clockwise_OutIn();

        void Run(void* pDlg) override;
    };

    class Spiral_UnClockwise_InOut : public ILinearTest
    {
    public:
        explicit Spiral_UnClockwise_InOut(float nLaps);
        ~Spiral_UnClockwise_InOut();

        void Run(void* pDlg) override;
    };

    class Spiral_UnClockwise_OutIn : public ILinearTest
    {
    public:
        explicit Spiral_UnClockwise_OutIn(float nLaps );
        ~Spiral_UnClockwise_OutIn();

        void Run(void* pDlg) override;
    };

    class LinearFactory
    {
    public:
        explicit LinearFactory(float laps) :m_Laps(laps) {}
        virtual ~LinearFactory() {}
        virtual ILinearTest* CreateLinearTest() = 0;
    protected:
        float m_Laps;
    };

    class SpiralCInOutFactory : public LinearFactory
    {
    public:
        explicit SpiralCInOutFactory(float nLaps) : LinearFactory(nLaps) {}
        ~SpiralCInOutFactory() {}
        ILinearTest* CreateLinearTest() override
        {
            return new Spiral_Clockwise_InOut(m_Laps);
        }
    };

    class SpiralCOutInFactory : public LinearFactory
    {
    public:
        explicit SpiralCOutInFactory(float nLaps) : LinearFactory(nLaps) {}
        ~SpiralCOutInFactory() {}
        ILinearTest* CreateLinearTest()
        {
            return new Spiral_Clockwise_OutIn(m_Laps);
        }
    };

    class SpiralUCInOutFactory : public LinearFactory
    {
    public:
        explicit SpiralUCInOutFactory(float nLaps) : LinearFactory(nLaps) {}
        ~SpiralUCInOutFactory() {}
        ILinearTest* CreateLinearTest() override
        {
            return new Spiral_UnClockwise_InOut(m_Laps);
        }
    };

    class SpiralUCOutInFactory : public LinearFactory
    {
    public:
        explicit SpiralUCOutInFactory(float nLaps) : LinearFactory(nLaps) {}
        ~SpiralUCOutInFactory() {}
        ILinearTest* CreateLinearTest() override
        {
            return new Spiral_UnClockwise_OutIn(m_Laps);
        }
    };

    static LinearFactory* CreateLinearFactory(int type, float laps)
    {
        if (type == 0)
        {
            return new SpiralCInOutFactory(laps);
        }
        else if (type == 1)
        {
            return new SpiralCOutInFactory(laps);
        }
        else if (type == 2)
        {
            return new SpiralUCInOutFactory(laps);
        }
        else if (type == 3)
        {
            return new SpiralUCOutInFactory(laps);
        }

        return nullptr;
    }
}
#endif //__HFST_LINEAR_TEST_TYPE_H__