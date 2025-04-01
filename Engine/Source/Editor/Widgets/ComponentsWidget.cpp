#include "ComponentsWidget.h"

#include "Runtime/Engine.h"
#include "Runtime/Entities/Entity.h"

#include "Runtime/Components/TransformComponent.h"
#include "Runtime/Components/MeshComponent.h"
#include "Runtime/Components/ShaderComponent.h"
#include "Runtime/Components/MaterialComponent.h"
#include "Runtime/Components/CameraComponent.h"
#include "Runtime/Components/SkeletonComponent.h"
#include "Runtime/Components/AnimationComponent.h"

#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QStringDecoder>
#include <QStringEncoder>

namespace Zongine {
    ComponentWidget::ComponentWidget(std::shared_ptr<Engine> engine, QWidget* parent /*= nullptr*/) {
        setColumnCount(2);
        setHeaderLabels({ "Property", "" });

        setAlternatingRowColors(true);

        m_Engine = engine;
        // UpdateComponents();
    }
    void ComponentWidget::UpdateComponents(uint64_t id) {
        auto entity = m_Engine->GetEntity(id);
        clear();
        if (entity.HasComponent<TransformComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Transform");

            auto& transform = entity.GetComponent<TransformComponent>();
            auto position = new QTreeWidgetItem(item);
            position->setText(0, "Position");

            AddVectorProperty(transform.Position, position);

            auto rotation = new QTreeWidgetItem(item);
            rotation->setText(0, "Rotation");

            AddVectorProperty(transform.Rotation, rotation);

            auto scale = new QTreeWidgetItem(item);
            scale->setText(0, "Scale");

            AddVectorProperty(transform.Scale, scale);

        }
        if (entity.HasComponent<MeshComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Mesh");

            auto mesh = entity.GetComponent<MeshComponent>();
            AddPathProperty(mesh.Path, item);
        }
        if (entity.HasComponent<ShaderComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Shader");

            auto shader = entity.GetComponent<ShaderComponent>();
            AddPathProperty(shader.Path, item);
        }
        if (entity.HasComponent<MaterialComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Material");
        }
        if (entity.HasComponent<CameraComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Camera");
        }
        if (entity.HasComponent<SkeletonComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Skeleton");
        }
        if (entity.HasComponent<AnimationComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Animation");
        }

        expandAll();
    }

    template<typename T>
    void ComponentWidget::AddVectorProperty(const T& vec, QTreeWidgetItem* parent) {
        QWidget* widget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        QSpinBox* xBox = new QSpinBox();
        QSpinBox* yBox = new QSpinBox();
        QSpinBox* zBox = new QSpinBox();

        xBox->setRange(-1000, 1000);
        yBox->setRange(-1000, 1000);
        zBox->setRange(-1000, 1000);

        xBox->setValue(vec.x);
        yBox->setValue(vec.y);
        zBox->setValue(vec.z);

        layout->addWidget(new QLabel("X:"));
        layout->addWidget(xBox);
        layout->addWidget(new QLabel("Y:"));
        layout->addWidget(yBox);
        layout->addWidget(new QLabel("Z:"));
        layout->addWidget(zBox);

        setItemWidget(parent, 1, widget);
    }

    void ComponentWidget::AddPathProperty(std::string path, QTreeWidgetItem* parent) {
        QLineEdit* lineEdit = new QLineEdit();

        QByteArray gbkBytes(path.c_str(), path.length());
        lineEdit->setText(QString::fromLocal8Bit(gbkBytes));
        lineEdit->setCursorPosition(0);

        setItemWidget(parent, 1, lineEdit);
    }
}