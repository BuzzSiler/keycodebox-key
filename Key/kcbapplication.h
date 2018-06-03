#ifndef KCBAPPLICATION_H
#define KCBAPPLICATION_H

namespace kcb
{
    typedef enum {ACCESS_NONE=-1, ACCESS_TAKE, ACCESS_RETURN} ACCESS_SELECTION;


    class Application
    {
        public:
            static void setAccessSelection(ACCESS_SELECTION selection);
            static ACCESS_SELECTION getAccessSelection();
            static void clearAccessSelection();
            static bool isTakeSelection();
            static bool isReturnSelection();

        private:
            static ACCESS_SELECTION m_selection;
    };

}

#endif // KCBAPPLICATION_H
