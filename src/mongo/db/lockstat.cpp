// lockstat.cpp

/**
*    Copyright (C) 2008 10gen Inc.
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "mongo/pch.h"
#include "lockstat.h"
#include "mongo/db/jsobj.h"

namespace mongo { 

    BSONObj LockStat::report() const { 
        BSONObjBuilder x;
        BSONObjBuilder y;
        x.append("R", (long long) timeLocked[0]);
        x.append("W", (long long) timeLocked[1]);
        if( timeLocked[2] || timeLocked[3] ) {
            x.append("r", (long long) timeLocked[2]);
            x.append("w", (long long) timeLocked[3]);
        }
        y.append("R", (long long) timeAcquiring[0]);
        y.append("W", (long long) timeAcquiring[1]);
        if( timeAcquiring[2] || timeAcquiring[3] ) {
            y.append("r", (long long) timeAcquiring[2]);
            y.append("w", (long long) timeAcquiring[3]);
        }
        return BSON(
            "timeLocked" << x.obj() << 
            "timeAcquiring" << y.obj()
        );
    }

    unsigned LockStat::mapNo(char type) {
        switch( type ) { 
        case 'R' : return 0;
        case 'W' : return 1;
        case 'r' : return 2;
        case 'w' : return 3;
        default: ;
        }
        fassert(16146,false);
        return 0;
    }

    LockStat::Acquiring::Acquiring(LockStat& _ls, char t) : ls(_ls) { 
        type = mapNo(t);
        dassert( type < N );
    }

    // note: we have race conditions on the following += 
    // hmmm....

    LockStat::Acquiring::~Acquiring() { 
        ls.timeAcquiring[type] += tmr.micros();
        if( type == 1 ) 
            ls.W_Timer.reset();
    }

    void LockStat::unlocking(char tp) { 
        unsigned type = mapNo(tp);
        if( type == 1 ) 
            timeLocked[type] += W_Timer.micros();
    }

}
