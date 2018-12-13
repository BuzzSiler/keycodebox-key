#ifndef KCBAPPLICATION_H
#define KCBAPPLICATION_H

#include <QString>

namespace kcb
{
    class Application
    {
        
        public:
            static void setTakeAccessSelection();
            static void setReturnAccessSelection();
            static QString getAccessSelection();
            static void clearAccessSelection();
            static bool isTakeSelection();
            static bool isReturnSelection();
            static void SetLastTransaction(QString username, QString locks, QString datetime);
            static QString GetLastTransactionAsString();


        private:
            typedef enum {ACCESS_NONE=-1, ACCESS_TAKE, ACCESS_RETURN} ACCESS_SELECTION;
            static ACCESS_SELECTION m_selection;
    };

}

#endif // KCBAPPLICATION_H
