/* ---- spdlog - COPYRIGHT NOTICE ----

    The MIT License (MIT)

    Copyright (c) 2016 Gabi Melman.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

*/

#include "log.h"

#include <QDebug>

void init_log( const char *file )
{
	auto file_logger = spdlog::basic_logger_mt( "fLog", file );
	spdlog::set_default_logger( file_logger );

	spdlog::set_pattern( "[%Y-%m-%dT%H:%M:%S.%e%z] %v" );

    // flush every type of log level
    spdlog::flush_on( spdlog::level::level_enum::trace );
}

void logError( const QString &error )
{
    qDebug() << "Error: " << error;
    spdlog::error( error.toStdString() );
}

void logInfo( const QString &info )
{
    qDebug() << "Info: " << info;
    spdlog::info( info.toStdString() );
}
