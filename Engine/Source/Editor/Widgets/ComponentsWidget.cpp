#include "ComponentsWidget.h"

#include "Runtime/Include/IEditorBridge.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDoubleValidator>
#include <QStringDecoder>
#include <QStringEncoder>

namespace Zongine {
    ComponentWidget::ComponentWidget(IEditorBridge& bridge, QWidget* parent /*= nullptr*/)
        : QTreeWidget(parent), m_Bridge(bridge)
    {
        setColumnCount(2);
        setHeaderLabels({ "Property", "" });
        setAlternatingRowColors(true);
    }

    void ComponentWidget::UpdateComponents(uint64_t id) {
        clear();

        auto components = m_Bridge.GetComponents(static_cast<uint32_t>(id));

        for (auto& comp : components) {
            auto* compItem = new QTreeWidgetItem(this);
            compItem->setText(0, QString::fromStdString(comp.Name));

            for (auto& prop : comp.Properties) {
                auto* propItem = new QTreeWidgetItem(compItem);
                propItem->setText(0, QString::fromStdString(prop.Name));

                std::visit([&](auto&& val) {
                    using T = std::decay_t<decltype(val)>;

                    if constexpr (std::is_same_v<T, std::array<float, 3>>) {
                        AddVectorProperty(val, propItem, id, comp.Name, prop.Name, prop.ReadOnly);
                    }
                    else if constexpr (std::is_same_v<T, float>) {
                        AddNumberProperty(val, propItem, id, comp.Name, prop.Name, prop.ReadOnly);
                    }
                    else if constexpr (std::is_same_v<T, int>) {
                        AddNumberProperty(val, propItem, prop.ReadOnly);
                    }
                    else if constexpr (std::is_same_v<T, uint32_t>) {
                        AddNumberProperty(val, propItem, prop.ReadOnly);
                    }
                    else if constexpr (std::is_same_v<T, bool>) {
                        AddBoolProperty(val, propItem, id, comp.Name, prop.Name, prop.ReadOnly);
                    }
                    else if constexpr (std::is_same_v<T, std::string>) {
                        AddStringProperty(val, propItem, prop.ReadOnly);
                    }
                }, prop.Value);
            }
        }

        expandAll();
    }

    void ComponentWidget::AddVectorProperty(const std::array<float, 3>& vec, QTreeWidgetItem* parent,
                                            uint64_t entityId, const std::string& componentName,
                                            const std::string& propertyName, bool readOnly)
    {
        QWidget* widget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        QLineEdit* xEdit = new QLineEdit(this);
        QLineEdit* yEdit = new QLineEdit(this);
        QLineEdit* zEdit = new QLineEdit(this);

        QDoubleValidator* validator = new QDoubleValidator(this);
        xEdit->setValidator(validator);
        yEdit->setValidator(validator);
        zEdit->setValidator(validator);

        xEdit->setText(QString::number(static_cast<double>(vec[0])));
        yEdit->setText(QString::number(static_cast<double>(vec[1])));
        zEdit->setText(QString::number(static_cast<double>(vec[2])));

        xEdit->setReadOnly(readOnly);
        yEdit->setReadOnly(readOnly);
        zEdit->setReadOnly(readOnly);

        if (!readOnly) {
            // Capture copies for the lambda
            std::string compName = componentName;
            std::string propName = propertyName;

            auto onEditFinished = [this, xEdit, yEdit, zEdit, entityId, compName, propName]() {
                std::array<float, 3> newVec = {
                    xEdit->text().toFloat(),
                    yEdit->text().toFloat(),
                    zEdit->text().toFloat()
                };
                m_Bridge.SetProperty(static_cast<uint32_t>(entityId), compName, propName,
                                     PropertyValue(newVec));
            };

            connect(xEdit, &QLineEdit::editingFinished, this, onEditFinished);
            connect(yEdit, &QLineEdit::editingFinished, this, onEditFinished);
            connect(zEdit, &QLineEdit::editingFinished, this, onEditFinished);
        }

        layout->addWidget(new QLabel(" X: "));
        layout->addWidget(xEdit);
        layout->addWidget(new QLabel(" Y: "));
        layout->addWidget(yEdit);
        layout->addWidget(new QLabel(" Z: "));
        layout->addWidget(zEdit);

        setItemWidget(parent, 1, widget);
    }

    void ComponentWidget::AddNumberProperty(float value, QTreeWidgetItem* parent,
                                            uint64_t entityId, const std::string& componentName,
                                            const std::string& propertyName, bool readOnly)
    {
        QLineEdit* edit = new QLineEdit(this);
        QDoubleValidator* validator = new QDoubleValidator(this);
        edit->setValidator(validator);
        edit->setText(QString::number(static_cast<double>(value)));
        edit->setReadOnly(readOnly);

        if (!readOnly) {
            std::string compName = componentName;
            std::string propName = propertyName;

            connect(edit, &QLineEdit::editingFinished, this, [this, edit, entityId, compName, propName]() {
                float newVal = edit->text().toFloat();
                m_Bridge.SetProperty(static_cast<uint32_t>(entityId), compName, propName,
                                     PropertyValue(newVal));
            });
        }

        setItemWidget(parent, 1, edit);
    }

    void ComponentWidget::AddNumberProperty(int value, QTreeWidgetItem* parent, bool readOnly) {
        QLineEdit* edit = new QLineEdit(this);
        edit->setValidator(new QDoubleValidator(this));
        edit->setText(QString::number(value));
        edit->setReadOnly(readOnly);
        setItemWidget(parent, 1, edit);
    }

    void ComponentWidget::AddNumberProperty(uint32_t value, QTreeWidgetItem* parent, bool readOnly) {
        QLineEdit* edit = new QLineEdit(this);
        edit->setValidator(new QDoubleValidator(this));
        edit->setText(QString::number(value));
        edit->setReadOnly(readOnly);
        setItemWidget(parent, 1, edit);
    }

    void ComponentWidget::AddBoolProperty(bool value, QTreeWidgetItem* parent,
                                          uint64_t entityId, const std::string& componentName,
                                          const std::string& propertyName, bool readOnly)
    {
        QCheckBox* checkBox = new QCheckBox(this);
        checkBox->setChecked(value);
        checkBox->setEnabled(!readOnly);

        if (!readOnly) {
            std::string compName = componentName;
            std::string propName = propertyName;

            connect(checkBox, &QCheckBox::toggled, this, [this, entityId, compName, propName](bool checked) {
                m_Bridge.SetProperty(static_cast<uint32_t>(entityId), compName, propName,
                                     PropertyValue(checked));
            });
        }

        setItemWidget(parent, 1, checkBox);
    }

    void ComponentWidget::AddStringProperty(const std::string& value, QTreeWidgetItem* parent, bool readOnly) {
        QLineEdit* lineEdit = new QLineEdit(this);

        QByteArray bytes(value.c_str(), static_cast<int>(value.length()));
        lineEdit->setText(QString::fromLocal8Bit(bytes));
        lineEdit->setCursorPosition(0);
        lineEdit->setReadOnly(readOnly);

        setItemWidget(parent, 1, lineEdit);
    }
}