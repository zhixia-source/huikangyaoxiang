const app = getApp();

Page({
  data: {
    med_name: '',
    med_num: '',
    med_time: '',
    med_DayFreq: '',
    med_numFreq: '',
    yao_add: '',
    yao_delete: '',
    result: '',
    currentMedCount: 0 // 初始化当前药品数量
  },

  // 绑定药品名称输入
  getMedName(e) {
    this.setData({
      med_name: e.detail.value
    });
  },

  // 绑定药品数量输入并转换为整型
  getMedNum(e) {
    this.setData({
      med_num: parseInt(e.detail.value, 10)
    });
  },

  // 绑定吃药时间输入
  getMedTime(e) {
    this.setData({
      med_time: e.detail.value
    });
  },

  // 绑定吃药次数输入并转换为整型
  getMedDayFreq(e) {
    this.setData({
      med_DayFreq: parseInt(e.detail.value, 10)
    });
  },

  // 绑定每次几颗输入并转换为整型
  getMedNumFreq(e) {
    this.setData({
      med_numFreq: parseInt(e.detail.value, 10)
    });
  },

  // 提交药品信息
  onSubmit() {
    const { med_name, med_num, med_time, med_DayFreq, med_numFreq } = this.data;
    const { currentMedCount } = app.globalData;

    // 存储药品信息到相应的medicineInfo1或medicineInfo2
    if (currentMedCount === 0) {
      app.globalData.medicineInfo1 = { med_name, med_num, med_time, med_DayFreq, med_numFreq };
      console.log('Saved medicineInfo1:', app.globalData.medicineInfo1);
      app.globalData.currentMedCount += 1;
    } else if (currentMedCount === 1) {
      app.globalData.medicineInfo2 = { med_name, med_num, med_time, med_DayFreq, med_numFreq };
      console.log('Saved medicineInfo2:', app.globalData.medicineInfo2);
      app.globalData.currentMedCount += 1;
    }
    this.setData({ currentMedCount: app.globalData.currentMedCount }); // 更新页面数据
    wx.navigateTo({
      url: '/pages/YaoXiang/YaoXiang'
    });
  },

  // 显示药品添加确认框并发送命令
  showCommandModal() {
    var that = this;
    const { currentMedCount } = app.globalData;
    // 检查药品数量是否已经达到最大值
    if (currentMedCount >= 2) {
      wx.showModal({
        title: '错误',
        content: '药箱已满，无法添加更多药品',
        showCancel: false
      });
      return;
    }
    wx.showModal({
      title: '向药箱发送药品信息',
      content: '确认吗',
      success: function (res) {
        if (res.confirm) {
          console.log('用户点击确定');
          const { med_name, med_num, med_time, med_DayFreq, med_numFreq } = that.data;
          // 构建命令参数
          var yao_add = `{"service_id": "medicine","command_name": "med_add","paras": {"med_name":"${med_name}","med_num":${med_num},"med_time":"${med_time}","med_DayFreq":${med_DayFreq},"med_numFreq":${med_numFreq}}}`;
          that.setData({ yao_add: yao_add });
          console.log("开始下发命令。。。");
          // 读取缓存中的token
          var token = wx.getStorageSync('token');
          // 发送命令请求
          wx.request({
            url: 'https://iotda.cn-north-4.myhuaweicloud.com/v5/iot/66597e1e2ca97925e05df9b7/devices/66597e1e2ca97925e05df9b7_123456/commands',
            data: yao_add,
            method: 'POST',
            header: {
              'content-type': 'application/json',
              'X-Auth-Token': token
            },
            success: function (res) {
              if (res.statusCode === 200) { // 确认命令成功下发
                console.log("命令下发成功");
                that.updateMedicineInfo(med_name, med_num, med_time, med_DayFreq, med_numFreq); // 调用更新药品信息函数
              } else {
                console.log("命令下发失败");
                wx.showToast({
                  title: '命令下发失败',
                  icon: 'none'
                });
              }
            },
            fail: function () {
              console.log("命令下发失败");
              wx.showToast({
                title: '请先获取token',
                icon: 'none'
              });
            },
            complete: function () {
              console.log("命令下发完成");
              that.setData({ result: '设备命令下发完成' });
            }
          });
        }
      }
    });
  },

  // 更新药品信息函数
  updateMedicineInfo(med_name, med_num, med_time, med_DayFreq, med_numFreq) {
    const { currentMedCount } = app.globalData;
    if (currentMedCount === 0) {
      app.globalData.medicineInfo1 = { med_name, med_num, med_time, med_DayFreq, med_numFreq };
      console.log('Updated medicineInfo1:', app.globalData.medicineInfo1);
      app.globalData.currentMedCount += 1;
    } else if (currentMedCount === 1) {
      app.globalData.medicineInfo2 = { med_name, med_num, med_time, med_DayFreq, med_numFreq };
      console.log('Updated medicineInfo2:', app.globalData.medicineInfo2);
      app.globalData.currentMedCount += 1;
    }
    this.setData({ currentMedCount: app.globalData.currentMedCount }); // 更新页面数据
    wx.navigateTo({
      url: '/pages/YaoXiang/YaoXiang'
    });
  },

  // 显示药品删除确认框并发送命令
  showCommandModal2() {
    var that = this;
    const { currentMedCount } = app.globalData;
    wx.showModal({
      title: '确认删除药品',
      content: '药箱减少药品',
      success: function (res) {
        if (res.confirm) {
          console.log('用户点击确定');
          // 构建命令参数
          var yao_delete = `{"service_id": "medicine","command_name": "med_delete","paras": {"delete": true}}`;
          that.setData({ yao_delete: yao_delete });
          console.log("开始下发命令。。。");
          // 读取缓存中的token
          var token = wx.getStorageSync('token');
          // 发送命令请求
          wx.request({
            url: 'https://iotda.cn-north-4.myhuaweicloud.com/v5/iot/66597e1e2ca97925e05df9b7/devices/66597e1e2ca97925e05df9b7_123456/commands',
            data: yao_delete,
            method: 'POST',
            header: {
              'content-type': 'application/json',
              'X-Auth-Token': token
            },
            success: function (res) {
              if (res.statusCode === 200) { // 确认命令成功下发
                console.log("下发命令成功");
                if (currentMedCount > 0) {
                  // 更新当前药品数量
                  app.globalData.currentMedCount -= 1;
                  // 同时清除相应的药品信息
                  if (currentMedCount === 1) {
                    app.globalData.medicineInfo1 = {};
                  } else if (currentMedCount === 2) {
                    app.globalData.medicineInfo2 = {};
                  }
                }
                that.setData({ currentMedCount: app.globalData.currentMedCount }); // 更新页面数据
              } else {
                console.log("命令下发失败");
                wx.showToast({
                  title: '命令下发失败',
                  icon: 'none'
                });
              }
            },
            fail: function () {
              console.log("命令下发失败");
              wx.showToast({
                title: '请先获取token',
                icon: 'none'
              });
            },
            complete: function () {
              console.log("命令下发完成");
              that.setData({ result: '设备命令下发完成' });
            }
          });
        }
      }
    });
  },

  onLoad: function () {
    // 在页面加载时，初始化药品信息
    this.setData({
      medicineInfo1: app.globalData.medicineInfo1 || {},
      medicineInfo2: app.globalData.medicineInfo2 || {},
      currentMedCount: app.globalData.currentMedCount
    });
  }
});
