import * as echarts from '../../ec-canvas/echarts';

const app = getApp();

Page({
  data: {
    medicineInfo1: {},
    medicineInfo2: {},
    med_YiChi1: 0,
    med_YiChi2: 0,
    ec1: {
      lazyLoad: true // 延迟加载
    },
    ec2: {
      lazyLoad: true // 延迟加载
    }
  },

  onLoad: function () {
    // 获取全局数据中的药品信息
    this.setData({
      medicineInfo1: app.globalData.medicineInfo1 || {},
      medicineInfo2: app.globalData.medicineInfo2 || {}
    });

    // 初始化图表
    this.initCharts();

    // 每分钟查询一次设备的影子数据
    this.startInterval();
  },

  onShow: function () {
    // 页面显示时更新图表数据
    this.updateCharts();
  },

  initCharts() {
    // 初始化图表1
    this.initChart1();

    // 初始化图表2
    this.initChart2();
  },

  updateCharts() {
    // 更新图表1
    this.updateChart1();

    // 更新图表2
    this.updateChart2();
  },

  initChart1() {
    const component = this.selectComponent('#ec1');
    component.init((canvas, width, height, dpr) => {
      this.chart1 = echarts.init(canvas, null, {
        width: width,
        height: height,
        devicePixelRatio: dpr
      });
      canvas.setChart(this.chart1);
      this.updateChart1(); // 初始化后立即更新图表数据
      return this.chart1;
    });
  },

  initChart2() {
    const component = this.selectComponent('#ec2');
    component.init((canvas, width, height, dpr) => {
      this.chart2 = echarts.init(canvas, null, {
        width: width,
        height: height,
        devicePixelRatio: dpr
      });
      canvas.setChart(this.chart2);
      this.updateChart2(); // 初始化后立即更新图表数据
      return this.chart2;
    });
  },

  updateChart1() {
    const { medicineInfo1, med_YiChi1 } = this.data;
    const option1 = this.buildOption(medicineInfo1, med_YiChi1);
    if (this.chart1) {
      this.chart1.setOption(option1);
    }
  },

  updateChart2() {
    const { medicineInfo2, med_YiChi2 } = this.data;
    const option2 = this.buildOption(medicineInfo2, med_YiChi2);
    if (this.chart2) {
      this.chart2.setOption(option2);
    }
  },

  buildOption(medicineInfo, med_YiChi) {
    const { med_name, med_num, med_DayFreq, med_numFreq } = medicineInfo;
    return {
      backgroundColor: "#ffffff",
      title: {
        text: `${med_name || '药品信息'} - 每日次数: ${med_DayFreq || 0}, 每次数量: ${med_numFreq || 0}`,
        left: 'center'
      },
      tooltip: {
        trigger: 'item'
      },
      series: [{
        name: '药品信息',
        type: 'pie',
        center: ['50%', '50%'],
        radius: ['30%', '60%'],
        data: [
          { value: med_num || 0, name: '剩余药品数量\n' + (med_num || 0) },
          { value: med_YiChi || 0, name: '已吃药品数量\n' + (med_YiChi || 0) }
        ],
        label: {
          rich: {
            a: {
              color: '#333',
              lineHeight: 16,
              align: 'center'
            }
          },
          formatter: function (params) {
            return `{a|${params.name}}`;
          }
        },
        emphasis: {
          itemStyle: {
            shadowBlur: 10,
            shadowOffsetX: 0,
            shadowColor: 'rgba(0, 0, 0, 0.5)'
          }
        }
      }]
    };
  },

  // 手动更新数据的按钮点击事件处理函数
  manualUpdateData() {
    // 获取全局数据中的药品信息，确保最新的数据
    this.setData({
      medicineInfo1: app.globalData.medicineInfo1 || {},
      medicineInfo2: app.globalData.medicineInfo2 || {}
    });

    console.log('Saved medicineInfo1:', this.data.medicineInfo1);
    console.log('Saved medicineInfo2:', this.data.medicineInfo2);

    // 更新图表数据
    this.updateCharts();
  },

  // 开始定时查询
  startInterval() {
    this.interval = setInterval(() => {
      this.checkDeviceShadow();
    }, 60000); // 每分钟查询一次
  },

  // 查询设备影子数据并更新状态
  checkDeviceShadow() {
    console.log("开始获取影子");
    const token = wx.getStorageSync('token');
    wx.request({
      url: 'https://iotda.cn-north-4.myhuaweicloud.com/v5/iot/66597e1e2ca97925e05df9b7/devices/66597e1e2ca97925e05df9b7_123456/shadow',
      data: '',
      method: 'GET',
      header: {
        'content-type': 'application/json',
        'X-Auth-Token': token
      },
      success: (res) => {
        console.log("获取影子成功", res);
        const deviceShadow = res.data.shadow[0].reported.properties; // 假设影子数据在 res.data.shadow[0].reported.properties 中

        const { medicineInfo1, medicineInfo2, med_YiChi1, med_YiChi2 } = this.data;

        let updateNeeded = true;

        if (deviceShadow.over1 && medicineInfo1.med_num > 0) {
          medicineInfo1.med_num -= medicineInfo1.med_numFreq;
          this.setData({
            med_YiChi1: med_YiChi1 + medicineInfo1.med_numFreq
          });
          updateNeeded = true;
        }

        if (deviceShadow.over2 && medicineInfo2.med_num > 0) {
          medicineInfo2.med_num -= medicineInfo2.med_numFreq;
          this.setData({
            med_YiChi2: med_YiChi2 + medicineInfo2.med_numFreq
          });
          updateNeeded = true;
        }

        if (updateNeeded) {
          // 更新图表数据
          this.setData({ medicineInfo1, medicineInfo2 });
          this.updateCharts();

          // 清空设备影子
          this.clearDeviceShadow();
        }
      },
      fail: () => {
        console.log("获取影子失败");
        console.log("请先获取token");
      },
      complete: () => {
        console.log("获取影子完成");
      }
    });
  },

  onUnload: function () {
    // 页面卸载时清除定时器
    if (this.interval) {
      clearInterval(this.interval);
    }
  }
});
