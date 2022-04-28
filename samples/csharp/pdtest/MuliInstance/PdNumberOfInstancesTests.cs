using LibPDBinding.Managed;
using NUnit.Framework;

namespace LibPDBindingTest.MultiInstance
{
    [TestFixture]
    public class PdNumberOfInstancesTests
    {
        [Test]
        public virtual void NoInstanceShouldBeZero()
        {
            Assert.AreEqual(0, Pd.NumberOfInstances);
        }
        [Test]
        public virtual void OneInstanceTest()
        {
            Pd instance = new Pd(2, 2, 44100);
            Assert.AreEqual(1, Pd.NumberOfInstances);
            instance.Dispose();
        }

        [Test]
        public virtual void TwoInstancesTest()
        {
            Pd instance1 = new Pd(2, 2, 44100);
            Pd instance2 = new Pd(2, 2, 44100);
            Assert.AreEqual(2, Pd.NumberOfInstances);
            instance2.Dispose();
            instance1.Dispose();
        }

        [Test]
        public virtual void DisposedInstanceTest()
        {
            Pd instance1 = new Pd(2, 2, 44100);
            Pd instance2 = new Pd(2, 2, 44100);
            instance2.Dispose();
            Assert.AreEqual(1, Pd.NumberOfInstances);
            instance1.Dispose();
        }
    }
}