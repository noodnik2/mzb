package controller

import (
	"fmt"

	"github.com/noodnik2/mzb/nextgen/backend/go/internal/model"
)

type MetaDict struct {
	model.MetaDataDict
}

func (md *MetaDict) SetType(key model.MetaDataKey, typ model.MetaDataType) error {
	md.MetaDataDict[key] = typ

	return nil
}

func (md *MetaDict) GetType(key model.MetaDataKey) (model.MetaDataType, error) {
	if mdt, ok := md.MetaDataDict[key]; ok {
		return mdt, nil
	}

	return "", fmt.Errorf("metadata type %q not found", key)
}
