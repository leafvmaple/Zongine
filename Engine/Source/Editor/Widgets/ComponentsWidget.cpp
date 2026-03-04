#include "ComponentsWidget.h"

#include "Runtime/Engine.h"
#include "Runtime/Entities/World.h"

#include "Runtime/Components/TransformComponent.h"
#include "Runtime/Components/MeshComponent.h"
#include "Runtime/Components/ShaderComponent.h"
#include "Runtime/Components/MaterialComponent.h"
#include "Runtime/Components/CameraComponent.h"
#include "Runtime/Components/SkeletonComponent.h"
#include "Runtime/Components/AnimationComponent.h"
#include "Runtime/Components/FlexibleComponent.h"

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
    }
    void ComponentWidget::UpdateComponents(uint64_t id) {
        auto& world = World::GetInstance();
        clear();
        if (world.Has<TransformComponent>(id)) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Transform");

            auto& transform = world.Get<TransformComponent>(id);
            auto position = new QTreeWidgetItem(item);
            position->setText(0, "Particles");

            AddVectorProperty(transform.Position, position);

            auto rotation = new QTreeWidgetItem(item);
            rotation->setText(0, "Rotation");

            AddVectorProperty(transform.Rotation, rotation);

            auto scale = new QTreeWidgetItem(item);
            scale->setText(0, "Scale");

            AddVectorProperty(transform.Scale, scale);

        }
        if (world.Has<MeshComponent>(id)) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Mesh");

            auto mesh = world.Get<MeshComponent>(id);
            auto path = new QTreeWidgetItem(item);
            path->setText(0, "Path");

            AddPathProperty(mesh.Path, path);
        }
        if (world.Has<ShaderComponent>(id)) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Shader");

            auto shader = world.Get<ShaderComponent>(id);
            auto paths = new QTreeWidgetItem(item);
            paths->setText(0, "Paths");

            for (int i = 0; i < shader.Paths.size(); i++) {
                auto path = new QTreeWidgetItem(paths);
                path->setText(0, QString("[%1]").arg(i));
                AddPathProperty(shader.Paths[i], path);
            }
        }
        if (world.Has<MaterialComponent>(id)) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Material");

            auto material = world.Get<MaterialComponent>(id);
            auto path = new QTreeWidgetItem(item);
            path->setText(0, "Path");

            AddPathProperty(material.Path, path);
        }
        if (world.Has<CameraComponent>(id)) {
            auto camera = world.Get<CameraComponent>(id);

            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Camera");

            auto perspective = new QTreeWidgetItem(item);
            perspective->setText(0, "AspectRatio");
            AddNumberProperty(camera.Perspective.fAspectRatio, perspective);

            auto fov = new QTreeWidgetItem(item);
            fov->setText(0, "Fov");
            AddNumberProperty(camera.Perspective.fFovAngleY, fov);
        }
        if (world.Has<SkeletonComponent>(id)) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Skeleton");

            auto skeleton = world.Get<SkeletonComponent>(id);
            auto path = new QTreeWidgetItem(item);
            path->setText(0, "Path");

            AddPathProperty(skeleton.Path, path);
        }
        if (world.Has<AnimationComponent>(id)) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Animation");

            auto animation = world.Get<AnimationComponent>(id);
            auto path = new QTreeWidgetItem(item);
            path->setText(0, "Path");

            AddPathProperty(animation.Path, path);
        }
        if (world.Has<FlexibleComponent>(id)) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Flexible");

            auto flexible = world.Get<FlexibleComponent>(id);
            auto drivers = new QTreeWidgetItem(item);
            drivers->setText(0, "Drivers");

            for (int i = 0; i < flexible.Drivers.size(); i++) {
                auto bone = new QTreeWidgetItem(drivers);
                bone->setText(0, QString("[%1]").arg(i));
                AddNumberProperty(flexible.Drivers[i].index, bone);
            }
        }

        expandAll();
    }

    template<typename T>
    void ComponentWidget::AddVectorProperty(const T& vec, QTreeWidgetItem* parent) {
        QWidget* widget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        QLineEdit* xEdit = new QLineEdit(this);
        QLineEdit* yEdit = new QLineEdit(this);
        QLineEdit* zEdit = new QLineEdit(this);

        QDoubleValidator* validator = new QDoubleValidator(this);

        zEdit->setValidator(validator);
        yEdit->setValidator(validator);
        xEdit->setValidator(validator);

        xEdit->setText(QString::number(vec.x));
        yEdit->setText(QString::number(vec.y));
        zEdit->setText(QString::number(vec.z));

        layout->addWidget(new QLabel(" X: "));
        layout->addWidget(xEdit);
        layout->addWidget(new QLabel(" Y: "));
        layout->addWidget(yEdit);
        layout->addWidget(new QLabel(" Z: "));
        layout->addWidget(zEdit);

        setItemWidget(parent, 1, widget);
    }

    template<typename T>
    void ComponentWidget::AddNumberProperty(T value, QTreeWidgetItem* parent) {
        QLineEdit* edit = new QLineEdit(this);
        QDoubleValidator* validator = new QDoubleValidator(this);

        edit->setValidator(validator);
        edit->setText(QString::number(value));

        setItemWidget(parent, 1, edit);
    }

    void ComponentWidget::AddPathProperty(std::string path, QTreeWidgetItem* parent) {
        QLineEdit* lineEdit = new QLineEdit();

        QByteArray gbkBytes(path.c_str(), path.length());
        lineEdit->setText(QString::fromLocal8Bit(gbkBytes));
        lineEdit->setCursorPosition(0);

        setItemWidget(parent, 1, lineEdit);
    }
}