//
// Copyright Copyright 2009-2022, AMT – The Association For Manufacturing Technology (“AMT”)
// All rights reserved.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

#pragma once

#include <boost/asio.hpp>

#include "logging.hpp"

namespace mtconnect::configuration {

  // Manages the boost asio context and allows for a syncronous
  // callback to execute when all the worker threads have stopped.
  class AsyncContext 
  {
  public:
    using SyncCallback = std::function<void(AsyncContext &context)>;
    
    auto &getContext() { return m_context; }
    operator boost::asio::io_context& () { return m_context; }
    
    void setThreadCount(int threads) { m_threadCount = threads; }
    
    
    void start()
    {
      m_running = true;
      do
      {
        for (int i = 0; i < m_threadCount; i++)
        {
          m_workers.emplace_back(std::thread([this]() { m_context.run(); }));
        }
        for (auto &w : m_workers)
        {
          w.join();
        }
        m_workers.clear();
        
        if (m_syncCallback)
        {
          m_syncCallback(*this);
          if (m_running)
          {
            m_syncCallback = nullptr;
            m_context.restart();
          }
        }
        
      } while (m_running);
    }
    
    void pause(SyncCallback callback)
    {
      m_syncCallback = callback;
      m_context.stop();
    }
    
    void stop()
    {
      m_running = false;
      m_context.stop();
    }
    
    void restart()
    {
      m_context.restart();
    }
    
  protected:
    boost::asio::io_context m_context;
    std::list<std::thread> m_workers;
    SyncCallback m_syncCallback;

    int m_threadCount = 1;
    bool m_running = false;
  };
    
}

