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
            auto path = new QTreeWidgetItem(item);
            path->setText(0, "Path");

            AddPathProperty(mesh.Path, path);
        }
        if (entity.HasComponent<ShaderComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Shader");

            auto shader = entity.GetComponent<ShaderComponent>();
            auto paths = new QTreeWidgetItem(item);
            paths->setText(0, "Paths");

            for (int i = 0; i < shader.Paths.size(); i++) {
                auto path = new QTreeWidgetItem(paths);
                path->setText(0, QString("[%1]").arg(i));
                AddPathProperty(shader.Paths[i], path);
            }
        }
        if (entity.HasComponent<MaterialComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Material");

            auto material = entity.GetComponent<MaterialComponent>();
            auto path = new QTreeWidgetItem(item);
            path->setText(0, "Path");

            AddPathProperty(material.Path, path);
        }
        if (entity.HasComponent<CameraComponent>()) {
            auto camera = entity.GetComponent<CameraComponent>();

            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Camera");

            auto perspective = new QTreeWidgetItem(item);
            perspective->setText(0, "AspectRatio");
            AddNumberProperty(camera.Perspective.fAspectRatio, perspective);

            auto fov = new QTreeWidgetItem(item);
            fov->setText(0, "Fov");
            AddNumberProperty(camera.Perspective.fFovAngleY, fov);
        }
        if (entity.HasComponent<SkeletonComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Skeleton");

            auto skeleton = entity.GetComponent<SkeletonComponent>();
            auto path = new QTreeWidgetItem(item);
            path->setText(0, "Path");

            AddPathProperty(skeleton.Path, path);
        }
        if (entity.HasComponent<AnimationComponent>()) {
            auto item = new QTreeWidgetItem(this);
            item->setText(0, "Animation");

            auto animation = entity.GetComponent<AnimationComponent>();
            auto path = new QTreeWidgetItem(item);
            path->setText(0, "Path");

            AddPathProperty(animation.Path, path);
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