//
// Copyright Copyright 2009-2021, AMT – The Association For Manufacturing Technology (“AMT”)
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
#include <map>
#include <unordered_map>

#include "component.hpp"
#include "data_item/data_item.hpp"
#include "utilities.hpp"

namespace mtconnect
{
  namespace adapter
  {
    class Adapter;
  }

  namespace device_model
  {
    class Device : public Component
    {
    public:
      // Constructor that sets variables from an attribute map
      Device(const std::string &name, entity::Properties &props);
      ~Device() override = default;

      auto getptr() const { return std::dynamic_pointer_cast<Device>(Entity::getptr()); }

      void initialize() override
      {
        Component::initialize();
        buildDeviceMaps(getptr());
        resolveReferences(getptr());
      }

      static entity::FactoryPtr getFactory();
      static entity::FactoryPtr getRoot();

      void setOptions(const ConfigOptions &options);

      // Add/get items to/from the device name to data item mapping
      void addDeviceDataItem(DataItemPtr dataItem);
      DataItemPtr getDeviceDataItem(const std::string &name) const;

      void addAdapter(adapter::Adapter *anAdapter) { m_adapters.emplace_back(anAdapter); }
      ComponentPtr getComponentById(const std::string &aId) const
      {
        auto comp = m_componentsById.find(aId);
        if (comp != m_componentsById.end())
          return comp->second.lock();
        else
          return nullptr;
      }
      void addComponent(ComponentPtr aComponent)
      {
        m_componentsById.insert(make_pair(aComponent->getId(), aComponent));
      }

      DevicePtr getDevice() const override { return getptr(); }

      // Return the mapping of Device to data items
      const auto &getDeviceDataItems() const { return m_deviceDataItemsById; }

      void addDataItem(DataItemPtr dataItem, entity::ErrorList &errors) override;

      std::vector<adapter::Adapter *> m_adapters;

      auto getMTConnectVersion() const { maybeGet<std::string>("mtconnectVersion"); }

      // Cached data items
      DataItemPtr getAvailability() const { return m_availability; }
      DataItemPtr getAssetChanged() const { return m_assetChanged; }
      DataItemPtr getAssetRemoved() const { return m_assetRemoved; }

      void setPreserveUuid(bool v) { m_preserveUuid = v; }
      bool preserveUuid() const { return m_preserveUuid; }

      void registerDataItem(DataItemPtr di);
      void registerComponent(ComponentPtr c) { m_componentsById[c->getId()] = c; }

    protected:
      void cachePointers(DataItemPtr dataItem);

    protected:
      bool m_preserveUuid { false };

      DataItemPtr m_availability;
      DataItemPtr m_assetChanged;
      DataItemPtr m_assetRemoved;

      // Mapping of device names to data items
      std::unordered_map<std::string, std::weak_ptr<data_item::DataItem>> m_deviceDataItemsByName;
      std::unordered_map<std::string, std::weak_ptr<data_item::DataItem>> m_deviceDataItemsById;
      std::unordered_map<std::string, std::weak_ptr<data_item::DataItem>> m_deviceDataItemsBySource;
      std::unordered_map<std::string, std::weak_ptr<Component>> m_componentsById;
    };

    using DevicePtr = std::shared_ptr<Device>;
  }  // namespace device_model
  using DevicePtr = std::shared_ptr<device_model::Device>;
}  // namespace mtconnect
