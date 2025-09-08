#include "BaseModel.h"

BaseModel::BaseModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // 初始化表头
    m_headers = {"时间", "代码", "价格", "数量", "方向", "成交额"};
}

QVariant BaseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }
    
    if (section >= 0 && section < m_headers.size()) {
        return m_headers[section];
    }
    
    return QVariant();
}