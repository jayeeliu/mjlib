<?php
error_reporting(E_ALL ^ E_NOTICE);

$opt = getopt('h:p:m:');
if (!$opt['h'] || !$opt['p']) {
	echo "Usage: php {$argv[0]} -h host -p port -m offline/online\n\n";
}

$host = $opt['h'] ? $opt['h'] : '172.16.139.60';
$port = $opt['p'] ? $opt['p'] : 20000;
$mode = $opt['m'] ? $opt['m'] : 'offline';

$kv = new KVStore($host, $port);
$table 	= 'tkv';
$key 	= 'key';
$value	= 'breuicnerofneromfoermfoermfoermfoemnroferofoerim';


if ($mode === 'offline') {
	echo "TEST {$host}:{$port} drop -> create -> get -> put -> get -> del -> get -> drop \n";
	assert($kv->drop($table) === true);
	assert($kv->create($table) === true);
	assert($kv->get($table, $key) === '');
	assert($kv->put($table, $key, $value) === true);
	assert($kv->get($table, $key) === $value);
	assert($kv->del($table, $key) === true);
	assert($kv->get($table, $key) === '');
	assert($kv->drop($table) === true);
} else {
	echo "TEST {$host}:{$port} del -> get -> put -> get -> del \n";
	assert($kv->del($table, $key) === true);
	assert($kv->get($table, $key) === '');
	assert($kv->put($table, $key, $value) === true);
	assert($kv->get($table, $key) === $value);
	assert($kv->del($table, $key) === true);
	assert($kv->get($table, $key) === '');

	echo "TEST {$host}:{$port} lpush -> lpush -> llen -> rpop -> llen -> rpop -> llen \n";
	$qkey = 'qkey';
	assert($kv->del($table, $qkey) === true);
	$v1 = $value.microtime(1);
	assert($kv->lpush($table, $qkey, $v1) === '1');
	$v2 = $value.microtime(1);
	assert($kv->lpush($table, $qkey, $v2) === '2');

	$v3 = $value.microtime(1);
	assert($kv->lpush($table, $qkey, $v3) === '3');

	assert($kv->rpop($table, $qkey) === $v1);
	assert($kv->rpop($table, $qkey) === $v2);
	assert($kv->llen($table, $qkey) === '1');
	assert($kv->rpop($table, $qkey) === $v3);
	assert($kv->rpop($table, $qkey) === '');
}
$kv->quit();



class KVStore {
	private $_fp = null;
	const IS_OK 	= 'OK',
		FLAG_LEN	= 128,	// 服务器端响应的标志位最大长度
		CMD_GET 	= "get %s %s\r\n",
		CMD_PUT		= "put %s %s %d\r\n%s",
		CMD_DEL 	= "del %s %s\r\n",
		CMD_CREATE 	= "create %s\r\n",
		CMD_DROP 	= "drop %s\r\n",

		CMD_RPOP 	= "rpop %s %s\r\n",
		CMD_LPUSH	= "lpush %s %s %d\r\n%s",
		CMD_LLEN 	= "llen %s %s\r\n",
		CMD_QUIT 	= "quit\r\n";

	public function __construct($host, $port=17879) {
		$this->_fp = fsockopen($host, $port, $errno, $errmsg, 3);
		if ($errno) {
			throw new Exception($errno, $errno);
		}
	}

	/**
	 * get 返回格式为
	 * 标志位OK空格数据长度（字节数）\r\n，或错误号空格错误消息\r\n
	 * 如果OK，并且有数据返回，剩下的都是数据
	 */
	public function get($table, $key) {
		fwrite($this->_fp, sprintf(self::CMD_GET, $table, $key));
		$len = $this->_length();
		return $len === false ? false : ($len > 0 ? stream_get_contents($this->_fp, $len) : '');
	}

	public function put($table, $key, $value) {
		fwrite($this->_fp, sprintf(self::CMD_PUT, $table, $key, strlen($value), $value));
		return self::_is_ok($this->_gets());
	}

	public function del($table, $key) {
		fwrite($this->_fp, sprintf(self::CMD_DEL, $table, $key));
		return self::_is_ok($this->_gets());
	}

	public function quit() {
		if ($this->_fp) {
			fwrite($this->_fp, sprintf(self::CMD_QUIT));
			fclose($this->_fp);
			$this->_fp = null;
		}
	}

/************************* offline only *******************************/
	public function create($table) {
		fwrite($this->_fp, sprintf(self::CMD_CREATE, $table));
		return self::_is_ok($this->_gets());
	}

	public function drop($table) {
		fwrite($this->_fp, sprintf(self::CMD_DROP, $table));
		return self::_is_ok($this->_gets());
	}
/**************************offline only end*****************************/

/************************** online only ********************************/
	public function rpop($table, $key) {
		fwrite($this->_fp, sprintf(self::CMD_RPOP, $table, $key));
		$len = $this->_length();
		return $len === false ? false : ($len > 0 ? fread($this->_fp, $len) : '');
	}

	public function lpush($table, $key, $value) {
		fwrite($this->_fp, sprintf(self::CMD_LPUSH, $table, $key, strlen($value), $value));
		$len = $this->_length();
		return $len === false ? false : ($len > 0 ? fread($this->_fp, $len) : '');
	}

	public function llen($table, $key) {
		fwrite($this->_fp, sprintf(self::CMD_LLEN, $table, $key));
		$len = $this->_length();
		return $len === false ? false : ($len > 0 ? fread($this->_fp, $len) : '');
	}

/************************** online only end ****************************/

	private function _gets() {
		if (!$this->_fp) {
			throw new Exception("connection off", 1);
		}
		return fgets($this->_fp, self::FLAG_LEN);
	}

	private function _length() {
		$flag = fgets($this->_fp, self::FLAG_LEN);

		if (!$this->_is_ok($flag)) {
			return false;
		}
		list(, $len) = explode(' ', $flag);
		return (int)$len;
	}

	private function _is_ok($ret) {
		var_dump($ret);
		return $ret && substr($ret, 0, 2) === self::IS_OK ? true : false;
	}

	public function __destruct() {
		if ($this->_fp) {
			fwrite($this->_fp, sprintf(self::CMD_QUIT));
			fclose($this->_fp);
		}
	}
}
