/*****************************************************************//**
 * \file   HFST_RandomPointTest.h
 * \brief  随机打点测试
 * 
 * \author river_yun
 * \date   December 2023
 *********************************************************************/
#include "../CommonModeNoiseRejectionDlg.h"
#include <vector>
#include <fstream>

namespace HFST
{
    class RandomPointTest
    {
        struct RandomPoint
        {
            RandomPoint() = default;
            RandomPoint(int xx, int yy, int idx)
                :x(xx), y(yy), index(idx) {}

            int x{ 0 };
            int y{ 0 };
            int index{ -1 };
        };
    public:
        RandomPointTest(){}

        RandomPointTest( size_t width, size_t height, size_t cntX = 50, size_t cntY = 2, size_t sectionNumber = 10)
            : m_TpWidth( width )
            , m_TpHeight( height )
            , m_CountX(cntX)
            , m_CountY(cntY)
            , m_SectionNumber(sectionNumber){}

        RandomPointTest(size_t width, size_t height)
            : m_TpWidth(width)
            , m_TpHeight(height) {}

        ~RandomPointTest() = default;

        void Init(CCommonModeNoiseRejectionDlg* pdlg);

        void SetSafeHeight(int height) { m_TestSafeHeight = height; }
        void SetTouchHeight(int height) { m_TestTouchHeight = height; }

        void NormalStart();

        void ReStart();

    private:
        void CreateResultDir();
        void CalculateRandomPoint();          // 计算随机的1000点坐标
        void Sort();
        void TestStep1();
        void TestStep2();
    private:
        CCommonModeNoiseRejectionDlg* m_pAppDlg{nullptr};

        size_t          m_TpWidth{ 100 };               // 宽(mm)
        size_t          m_TpHeight{ 60 };               // 高(mm)
        size_t          m_CountX{ 50 };                 // X方向数量
        size_t          m_CountY{ 2 };                  // Y方向数量
        size_t          m_SectionNumber{ 10 };          // 分区的数量
        bool            m_ReStart{false};               // 是否在掉线之后，继续之前的打点测试

        std::ofstream   m_FileAll;
        std::ofstream   m_FileStep1;
        std::ofstream   m_FileStep2;

        std::vector<std::vector<RandomPoint>>   m_vecPoint;  // 所有待测点
        std::string     m_strResultDir;

        std::vector<RandomPoint>                m_vecStep1Failed;
        std::vector<RandomPoint>                m_vecStep2Failed;

        int             m_TestSafeHeight;
        int             m_TestTouchHeight;
    };
}