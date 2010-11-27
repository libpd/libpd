/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core.utils;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.easymock.EasyMock;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.puredata.core.PdBase;


/**
 * 
 * Tests for {@link PdDispatcher}.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com) 
 *
 */
public class PdDispatcherTest {

	private PdDispatcher dispatcher;
	private PdListener listener1, listener2;
	
	@Before
	public void setUp() {
		listener1 = EasyMock.createStrictMock(PdListener.class);
		listener2 = EasyMock.createStrictMock(PdListener.class);
		dispatcher = new PdDispatcher() {
			@Override
			public void print(String s) {
				// Nothing to test here.
			}
		};
	}
	
	@After
	public void tearDown() {
		dispatcher.release();
		PdBase.release();
	}
	
	@Test
	public void testSubscription() {
		PdBase.setReceiver(dispatcher);
		assertFalse(PdBase.exists("foo"));
		assertFalse(PdBase.exists("bar"));
		dispatcher.addListener("foo", listener1);
		assertTrue(PdBase.exists("foo"));
		assertFalse(PdBase.exists("bar"));
		dispatcher.addListener("bar", listener2);
		assertTrue(PdBase.exists("foo"));
		assertTrue(PdBase.exists("bar"));
		dispatcher.addListener("foo", listener2);
		assertTrue(PdBase.exists("foo"));
		assertTrue(PdBase.exists("bar"));
		dispatcher.removeListener("foo", listener1);
		assertTrue(PdBase.exists("foo"));
		assertTrue(PdBase.exists("bar"));
		dispatcher.removeListener("bar", listener1);
		assertTrue(PdBase.exists("foo"));
		assertTrue(PdBase.exists("bar"));
		dispatcher.removeListener("bar", listener2);
		assertTrue(PdBase.exists("foo"));
		assertFalse(PdBase.exists("bar"));
		dispatcher.removeListener("foo", listener2);
		assertFalse(PdBase.exists("foo"));
		assertFalse(PdBase.exists("bar"));
	}
	
	@Test
	public void testCallbacks() {
		dispatcher.addListener("foo", listener1);
		dispatcher.addListener("bar", listener2);
		listener1.receiveBang();
		listener2.receiveBang();
		listener1.receiveFloat(0f);
		listener2.receiveFloat(1.5f);
		listener1.receiveSymbol("hund");
		listener2.receiveSymbol("katze");
		listener1.receiveList(0f, "call", 3f, "me", 7f, "ishmael");
		listener2.receiveList("test", 1f, 2f);
		listener1.receiveMessage("dest", -3f);
		listener2.receiveMessage("zzz", "eggs");
		EasyMock.replay(listener1, listener2);
		dispatcher.receiveBang("foo");
		dispatcher.receiveBang("bar");
		dispatcher.receiveFloat("foo", 0f);
		dispatcher.receiveFloat("bar", 1.5f);
		dispatcher.receiveSymbol("foo", "hund");
		dispatcher.receiveSymbol("bar", "katze");
		dispatcher.receiveList("foo", 0f, "call", 3f, "me", 7f, "ishmael");
		dispatcher.receiveList("bar", "test", 1f, 2f);
		dispatcher.receiveMessage("foo", "dest", -3f);
		dispatcher.receiveMessage("bar", "zzz", "eggs");
		dispatcher.receiveBang("spam");
		EasyMock.verify(listener1, listener2);
	}
	
	@Test
	public void testMultipleListeners() {
		dispatcher.addListener("spam", listener1);
		dispatcher.addListener("spam", listener2);
		listener1.receiveBang();
		listener2.receiveBang();
		EasyMock.replay(listener1, listener2);
		dispatcher.receiveBang("spam");
		EasyMock.verify(listener1, listener2);
	}
}
